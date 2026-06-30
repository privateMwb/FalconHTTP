#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace FalconHTTP::Core {

    class Socket {
    public:

        // Identity
        static constexpr int invalidHandle = -1;

    private:

        // Storage
        int fd_ = invalidHandle;

    public:

        // Constructors % Destructor
        Socket()                 noexcept;
        explicit Socket(int fd)  noexcept;
        ~Socket()                noexcept;

        Socket(const Socket&) = delete;
        Socket& operator=(const Socket&) = delete;

        Socket(Socket&& other)             noexcept;
        Socket& operator=(Socket&& other)  noexcept;

        // Creation
        [[nodiscard]] static Socket createTcp() noexcept;

        // Socket Options
        [[nodiscard]] bool setReuseAddr(bool enable)    noexcept;
        [[nodiscard]] bool setNonBlocking(bool enable)  noexcept;
        [[nodiscard]] bool setNoDelay(bool enable)      noexcept;

        // I/O
        [[nodiscard]] std::ptrdiff_t send(const void* data, std::size_t length)  noexcept;
        [[nodiscard]] std::ptrdiff_t receive(void* buffer, std::size_t length)   noexcept;

        // Lifecycle
        void close() noexcept;
        [[nodiscard]] bool isValid() const noexcept;

        // Accessories
        [[nodiscard]] int handle() const noexcept;
    };

} // namespace FalconHTTP::Core