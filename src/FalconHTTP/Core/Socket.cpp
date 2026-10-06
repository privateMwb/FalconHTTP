/**
 * @file Socket.cpp
 * @brief Socket implementation.
 *
 * Contains the implementation of Socket's construction, connecting,
 * socket options, I/O, and lifecycle management across POSIX and Windows.
 */

// ============================================================
// Implementation for FalconHTTP::Core::Socket.
// ============================================================
//
//  Sections:
//   1. Windows Initialization
//   2. Constructors & Destructor
//   3. Creation
//   4. Connection
//   5. Socket Options
//   6. I/O
//   7. Lifecycle
//   8. Accessories
//
// ============================================================

// clang-format off
#include <FalconHTTP/Core/Socket.h> // Socket (own header)

#include <algorithm> // std::min, std::max - clamping timeouts to the OS's integer ranges
#include <limits>    // std::numeric_limits<int>::max - poll() timeout ceiling

#ifdef _WIN32
#include <winsock2.h> // socket, send, recv, setsockopt, closesocket, shutdown
#include <ws2tcpip.h> // (additional Winsock helpers)
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>  // socket, send, recv, setsockopt, shutdown
#include <netinet/in.h>  // sockaddr_in
#include <netinet/tcp.h> // TCP_NODELAY
#include <arpa/inet.h>   // htons
#include <unistd.h>      // close
#include <fcntl.h>       // fcntl (non-blocking mode)
#include <sys/ioctl.h>   // ioctl (FIONBIO fallback)
#include <cerrno>        // errno, EINTR, EINPROGRESS
#include <poll.h>        // poll, pollfd - waiting for a non-blocking connect()
#endif
// clang-format on

namespace FalconHTTP::Core {

// ============================================================
//  Section 1 — Windows Initialization
// ============================================================

#ifdef _WIN32
namespace {
struct WinsockInitializer {
    WinsockInitializer() {
        WSADATA data;
        WSAStartup(MAKEWORD(2, 2), &data);
    }

    ~WinsockInitializer() {
        WSACleanup();
    }
};

WinsockInitializer winsockInitializer;
} // namespace
#endif

// ============================================================
//  Section 2 — Constructors & Destructor
// ============================================================

Socket::Socket() noexcept : fd_(invalidHandle) {}

Socket::Socket(int id) noexcept : fd_(id) {}

Socket::~Socket() noexcept {
    close();
}

Socket::Socket(Socket&& other) noexcept
    : fd_(other.fd_.exchange(invalidHandle, std::memory_order_acq_rel)) {}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        close();
        fd_.store(other.fd_.exchange(invalidHandle, std::memory_order_acq_rel),
                  std::memory_order_release);
    }
    return *this;
}

// ============================================================
//  Section 3 — Creation
// ============================================================

Socket Socket::createTcp() noexcept {
    int fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

#if !defined(_WIN32) && !defined(MSG_NOSIGNAL) && defined(SO_NOSIGPIPE)
    if (fd != invalidHandle) {
        int value = 1;
        ::setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &value, sizeof(value));
    }
#endif

    return Socket(fd);
}

// ============================================================
//  Section 4 — Connection
// ============================================================

namespace {

// Fills @p out from a dotted-decimal IPv4 address and port. Hostnames
// are deliberately not resolved here - callers pass numeric addresses.
bool fillAddress(const std::string& address, uint16_t port, sockaddr_in& out) noexcept {
    out = sockaddr_in{};
    out.sin_family = AF_INET;
    out.sin_port = htons(port);

    return ::inet_pton(AF_INET, address.c_str(), &out.sin_addr) == 1;
}

// Waits up to @p timeoutMs for a non-blocking connect() already in
// flight on @p fd to finish, then reads its outcome from SO_ERROR.
bool awaitConnect(int fd, int timeoutMs) noexcept {
#ifdef _WIN32
    WSAPOLLFD pollEntry{};
    pollEntry.fd = static_cast<SOCKET>(fd);
    pollEntry.events = POLLWRNORM;

    if (::WSAPoll(&pollEntry, 1, timeoutMs) <= 0)
        return false;

    int error = 0;
    int length = sizeof(error);
    if (::getsockopt(static_cast<SOCKET>(fd), SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error),
                     &length) != 0)
        return false;

    return error == 0;
#else
    pollfd pollEntry{};
    pollEntry.fd = fd;
    pollEntry.events = POLLOUT;

    int ready;
    do {
        ready = ::poll(&pollEntry, 1, timeoutMs);
    } while (ready < 0 && errno == EINTR);

    if (ready <= 0)
        return false; // Timed out (0) or poll() itself failed (-1).

    int error = 0;
    socklen_t length = sizeof(error);
    if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &length) != 0)
        return false;

    return error == 0;
#endif
}

} // namespace

bool Socket::connect(const std::string& address, uint16_t port) noexcept {
    const int fd = fd_.load(std::memory_order_acquire);
    sockaddr_in target;

    if (fd == invalidHandle || !fillAddress(address, port, target))
        return false;

    return ::connect(fd, reinterpret_cast<const sockaddr*>(&target), sizeof(target)) == 0;
}

bool Socket::connect(const std::string& address, uint16_t port,
                     std::chrono::milliseconds timeout) noexcept {
    const int fd = fd_.load(std::memory_order_acquire);
    sockaddr_in target;

    if (fd == invalidHandle || timeout.count() <= 0 || !fillAddress(address, port, target))
        return false;

    if (!setNonBlocking(true))
        return false;

    bool connected = false;
    const int result = ::connect(fd, reinterpret_cast<const sockaddr*>(&target), sizeof(target));

    if (result == 0) {
        connected = true; // Completed immediately (loopback often does).
    } else {
#ifdef _WIN32
        const bool inProgress = ::WSAGetLastError() == WSAEWOULDBLOCK;
#else
        const bool inProgress = errno == EINPROGRESS;
#endif
        if (inProgress) {
            const auto clamped =
                std::min<long long>(timeout.count(), (std::numeric_limits<int>::max)());
            connected = awaitConnect(fd, static_cast<int>(clamped));
        }
    }

    // Documented contract: blocking mode on return, success or not.
    (void)setNonBlocking(false);
    return connected;
}

// ============================================================
//  Section 5 — Socket Options
// ============================================================

bool Socket::setReuseAddr(bool enable) noexcept {
    int value = enable ? 1 : 0;

    return ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&value),
                        sizeof(value)) == 0;
}

bool Socket::setNonBlocking(bool enable) noexcept {
#ifdef _WIN32
    u_long mode = enable ? 1 : 0;
    return ::ioctlsocket(fd_, FIONBIO, &mode) == 0;
#else
    int flags = ::fcntl(fd_, F_GETFL, 0);

    if (flags == -1)
        return false;

    if (enable)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    return ::fcntl(fd_, F_SETFL, flags) == 0;
#endif
}

bool Socket::setNoDelay(bool enable) noexcept {
    int value = enable ? 1 : 0;

#ifdef _WIN32
    return ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&value),
                        sizeof(value)) == 0;
#else
    return ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value)) == 0;
#endif
}

bool Socket::setReceiveTimeout(std::chrono::milliseconds timeout) noexcept {
    const long long ms = std::max<long long>(timeout.count(), 0);

#ifdef _WIN32
    const DWORD value =
        static_cast<DWORD>(std::min<long long>(ms, (std::numeric_limits<int>::max)()));
    return ::setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&value),
                        sizeof(value)) == 0;
#else
    timeval value{};
    value.tv_sec = static_cast<time_t>(ms / 1000);
    value.tv_usec = static_cast<suseconds_t>((ms % 1000) * 1000);
    return ::setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, &value, sizeof(value)) == 0;
#endif
}

bool Socket::setSendTimeout(std::chrono::milliseconds timeout) noexcept {
    const long long ms = std::max<long long>(timeout.count(), 0);

#ifdef _WIN32
    const DWORD value =
        static_cast<DWORD>(std::min<long long>(ms, (std::numeric_limits<int>::max)()));
    return ::setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&value),
                        sizeof(value)) == 0;
#else
    timeval value{};
    value.tv_sec = static_cast<time_t>(ms / 1000);
    value.tv_usec = static_cast<suseconds_t>((ms % 1000) * 1000);
    return ::setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO, &value, sizeof(value)) == 0;
#endif
}

// ============================================================
//  Section 6 — I/O
// ============================================================

std::ptrdiff_t Socket::send(const void* data, std::size_t length) noexcept {
    std::ptrdiff_t result;

    do {
#if defined(MSG_NOSIGNAL)
        result = ::send(fd_, reinterpret_cast<const char*>(data), length, MSG_NOSIGNAL);
#else
        result = ::send(fd_, reinterpret_cast<const char*>(data), length, 0);
#endif
        // A signal delivered while blocked in send()/recv() (SIGCHLD
        // from thread/process churn elsewhere in the process, timer
        // signals, etc.) interrupts the syscall with EINTR - this is
        // not a real error, just "nothing was sent yet, try again."
        // Only meaningful on POSIX; Winsock has no equivalent for a
        // blocking call interrupted by a signal.
#ifndef _WIN32
    } while (result < 0 && errno == EINTR);
#else
    } while (false);
#endif

    return result;
}

std::ptrdiff_t Socket::receive(void* buffer, std::size_t length) noexcept {
    std::ptrdiff_t result;

    do {
        result = ::recv(fd_, reinterpret_cast<char*>(buffer), length, 0);
#ifndef _WIN32
    } while (result < 0 && errno == EINTR);
#else
    } while (false);
#endif

    return result;
}

// ============================================================
//  Section 7 — Lifecycle
// ============================================================

void Socket::close() noexcept {
    int fd = fd_.exchange(invalidHandle, std::memory_order_acq_rel);

    if (fd == invalidHandle)
        return;

#ifdef _WIN32
    // Wake any blocking send()/recv() before closing.
    ::shutdown(fd, SD_BOTH);
    ::closesocket(fd);
#else
    // Wake any blocking send()/recv() before closing.
    ::shutdown(fd, SHUT_RDWR);
    ::close(fd);
#endif
}

bool Socket::isValid() const noexcept {
    return fd_.load(std::memory_order_acquire) != invalidHandle;
}

// ============================================================
//  Section 8 — Accessories
// ============================================================

int Socket::handle() const noexcept {
    return fd_.load(std::memory_order_acquire);
}

} // namespace FalconHTTP::Core