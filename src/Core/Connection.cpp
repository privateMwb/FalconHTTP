#include "Core/Connection.h"

namespace FalconHTTP::Core {

    // Constructor
    Connection::Connection(Socket socket) noexcept
        : socket_(std::move(socket)) {
    }

    // Core API
    bool Connection::sendAll(const void* data, std::size_t length) noexcept {
        const auto* bytes = static_cast<const std::byte*>(data);
        std::size_t totalSent = 0;

        while (totalSent < length) {
            std::ptrdiff_t sent = socket_.send(bytes + totalSent, length - totalSent);
            if (sent == 0) return false;
            totalSent += static_cast<std::size_t>(sent);
        }

        return true;
    }

    std::string Connection::receiveAvailable(std::size_t chunkSize) noexcept {
        std::string     buffer(chunkSize, '\0');
        std::ptrdiff_t  received = socket_.receive(buffer.data(), chunkSize);

        if (received <= 0) return {};

        buffer.resize(static_cast<std::size_t>(received));
        return buffer;
    }

    void Connection::close()        noexcept { socket_.close(); }
    bool Connection::isOpen() const noexcept { return socket_.isValid(); }

} // namespace FalconHTTP::Core