#pragma once

#include "Socket.h"

#include <cstdint>

namespace FalconHTTP::Core {

    class Listener {
    public:

        // Identity
        static constexpr int defaultBacklog = 128;

    private:

        // Storage
        Socket socket_;
        uint16_t port_ = 0;

    public:

        // Constructors & Destructor
        Listener() noexcept = default;
        ~Listener() noexcept = default;

        Listener(const Listener&) = delete;
        Listener& operator=(const Listener&) = delete;

        Listener(Listener&& other) noexcept = default;
        Listener& operator=(Listener&& other) noexcept = default;

        // Core API
        [[nodiscard]] bool      start(uint16_t port, int backlog = defaultBacklog) noexcept;
        [[nodiscard]] Socket    accept()       noexcept;
        void                    stop()         noexcept;
        [[nodiscard]] bool      isListening()  const noexcept;
        [[nodiscard]] uint16_t  port()         const noexcept;
    };

} // namespace FalconHTTP::Core