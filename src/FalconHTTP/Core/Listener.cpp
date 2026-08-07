/**
 * @file Listener.cpp
 * @brief Listener implementation.
 */

// clang-format off
#include <FalconHTTP/Core/Socket.h>   // Socket
#include <FalconHTTP/Core/Listener.h> // Listener (own header)

#ifdef _WIN32
#include <winsock2.h>  // sockaddr_in, bind/listen/accept
#include <ws2tcpip.h>  // (additional Winsock helpers)
#else
#include <arpa/inet.h>   // htons, INADDR_ANY
#include <netinet/in.h>  // sockaddr_in
#include <sys/socket.h>  // bind, listen, accept
#endif
// clang-format on

namespace FalconHTTP::Core {

// Core API
bool Listener::start(uint16_t port, int backlog) noexcept {
    socket_ = Socket::createTcp();

    if (!socket_.isValid())
        return false;
    if (!socket_.setReuseAddr(true))
        return false;

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (::bind(socket_.handle(), reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
        socket_.close();
        return false;
    }

    if (::listen(socket_.handle(), backlog) != 0) {
        socket_.close();
        return false;
    }

    port_ = port;
    return true;
}

Socket Listener::accept() noexcept {
    int clientFd = ::accept(socket_.handle(), nullptr, nullptr);
    return Socket(clientFd);
}

void Listener::stop() noexcept {
    socket_.close();
    port_ = 0;
}

bool Listener::isListening() const noexcept {
    return socket_.isValid();
}

uint16_t Listener::port() const noexcept {
    return port_;
}

} // namespace FalconHTTP::Core
