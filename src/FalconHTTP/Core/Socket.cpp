/**
 * @file Socket.cpp
 * @brief Socket implementation.
 *
 * Contains the implementation of Socket's construction, socket options,
 * I/O, and lifecycle management across POSIX and Windows.
 */

// ============================================================
// Implementation for FalconHTTP::Core::Socket.
// ============================================================
//
//  Sections:
//   1. Windows Initialization
//   2. Constructors & Destructor
//   3. Creation
//   4. Socket Options
//   5. I/O
//   6. Lifecycle
//   7. Accessories
//
// ============================================================

// clang-format off
#include <FalconHTTP/Core/Socket.h> // Socket (own header)

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

Socket::Socket(Socket&& other) noexcept : fd_(other.fd_) {
    other.fd_ = invalidHandle;
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        close();
        fd_ = other.fd_;
        other.fd_ = invalidHandle;
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
//  Section 4 — Socket Options
// ============================================================

bool Socket::setReuseAddr(bool enable) noexcept {
    int value = enable ? 1 : 0;

    return ::setsockopt(fd_,
                        SOL_SOCKET,
                        SO_REUSEADDR,
                        reinterpret_cast<const char*>(&value),
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
    return ::setsockopt(fd_,
                        IPPROTO_TCP,
                        TCP_NODELAY,
                        reinterpret_cast<const char*>(&value),
                        sizeof(value)) == 0;
#else
    return ::setsockopt(fd_,
                        IPPROTO_TCP,
                        TCP_NODELAY,
                        &value,
                        sizeof(value)) == 0;
#endif
}

// ============================================================
//  Section 5 — I/O
// ============================================================

std::ptrdiff_t Socket::send(const void* data, std::size_t length) noexcept {
#if defined(MSG_NOSIGNAL)
    return ::send(fd_, reinterpret_cast<const char*>(data), length, MSG_NOSIGNAL);
#else
    return ::send(fd_, reinterpret_cast<const char*>(data), length, 0);
#endif
}

std::ptrdiff_t Socket::receive(void* buffer, std::size_t length) noexcept {
    return ::recv(fd_, reinterpret_cast<char*>(buffer), length, 0);
}

// ============================================================
//  Section 6 — Lifecycle
// ============================================================

void Socket::close() noexcept {
    if (fd_ == invalidHandle)
        return;

#ifdef _WIN32
    // Wake any blocking send()/recv() before closing.
    ::shutdown(fd_, SD_BOTH);
    ::closesocket(fd_);
#else
    // Wake any blocking send()/recv() before closing.
    ::shutdown(fd_, SHUT_RDWR);
    ::close(fd_);
#endif

    fd_ = invalidHandle;
}

bool Socket::isValid() const noexcept {
    return fd_ != invalidHandle;
}

// ============================================================
//  Section 7 — Accessories
// ============================================================

int Socket::handle() const noexcept {
    return fd_;
}

} // namespace FalconHTTP::Core