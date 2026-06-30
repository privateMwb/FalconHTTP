#include "Core/Socket.h"

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#endif

//#include <ws2tcpip.h>

namespace FalconHTTP::Core {

#ifdef _WIN32
    // Windows Initialization
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
        WinsockInitializer WinsockInitializer;
    }
#endif
    // Constructor
    Socket::Socket() noexcept
        : fd_(invalidHandle) {
    }

    Socket::Socket(int id) noexcept
        : fd_(id) {
    }

    Socket::~Socket() noexcept { close(); }

    Socket::Socket(Socket&& other) noexcept
        : fd_(other.fd_)
    {
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

    // Creation
    Socket Socket::createTcp() noexcept {
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        return Socket(fd);
    }

    // Socket Options
    bool Socket::setReuseAddr(bool enable) noexcept {
        int value = enable ? 1 : 0;
        return ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR,
            reinterpret_cast<const char*>(&value), sizeof(value)) == 0;
    }

    bool Socket::setNonBlocking(bool enable) noexcept {
        u_long mode = enable ? 1 : 0;
        return ioctlsocket(fd_, FIONBIO, &mode) == 0;

        /*int flags = ::fcntl(fd_, F_GETFL, 0);
        if (flags == -1) {
            return false;
        }
        flags = enable ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
        return ::fcntl(fd_, F_SETFL, flags) == 0;
        */
    }

    bool Socket::setNoDelay(bool enable) noexcept {
        int value = enable ? 1: 0;
        return ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY,
        reinterpret_cast<const char*>(&value), sizeof(value)) == 0;
    }

    // I/O
    std::ptrdiff_t Socket::send(const void* data, std::size_t length)  noexcept {
        return ::send(fd_, reinterpret_cast<const char*>(data), length, 0);
    }

    std::ptrdiff_t Socket::receive(void* buffer, std::size_t length)   noexcept {
        return ::recv(fd_, reinterpret_cast<char*>(buffer), length, 0);
    }

    // Lifecycle
    void Socket::close() noexcept {
        if (fd_ != invalidHandle) {
#ifdef _WIN32
            ::closesocket(fd_);
#else
            ::close(fd_);
#endif
            fd_ = invalidHandle;
        }
    }

    bool Socket::isValid() const noexcept {
        return fd_ != invalidHandle;
    }

    // Accessories
    int Socket::handle() const noexcept {
        return fd_;
    }

}; // namespace FalconHTTP::Core