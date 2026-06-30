#pragma once

#include "Socket.h"

#include <cstddef>
#include <string>

namespace FalconHTTP::Core {

    class Connection {

    public:

        // Identity
        static constexpr std::size_t DefaultReadChunk = 4096;

    private:

        // Storage
        Socket socket_;

    public:

        // Constructors & Destructor
        Connection() noexcept = default;
        explicit Connection(Socket socket) noexcept;
        ~Connection() noexcept = default;

        Connection(const Connection&)             = delete;
        Connection& operator=(const Connection&)  = delete;

        Connection(Connection&& other)             noexcept = default;
        Connection& operator=(Connection&& other)  noexcept = default;
        
        // Core API
        [[nodiscard]] bool         sendAll(const void* data, std::size_t length)               noexcept;
        [[nodiscard]] std::string  receiveAvailable(std::size_t chunkSize = DefaultReadChunk)  noexcept;
        void                       close() noexcept;
        [[nodiscard]] bool         isOpen() const noexcept;
    };

} // namespace FalconHTTP::Core