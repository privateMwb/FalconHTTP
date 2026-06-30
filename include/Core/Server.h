#pragma once

#include "Connection.h"
#include "Listener.h"
#include "Routing/Router.h"
#include "Middleware/Middleware.h"

#include "ThreadPool.h"
#include "Vector.h"

#include <cstdint>

namespace FalconHTTP::Core {

    class Server {
        public:

        // Identity
        static constexpr std::size_t DefaultReadChunk = 4096;

        private:
        
        // Storage
        Listener                                     listener_;
        Routing::Router*                             router_ = nullptr;
        ThreadPool                                   pool_;
        VectorPro::Vector<Middleware::MiddlewareFn>  middleware_;
        bool                                         running_ = false;

        public:

        // Constructors
        Server() noexcept = default;
        explicit Server(Routing::Router& router, std::size_t threadCount) noexcept;

        ~Server() noexcept = default;

        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;

        // Middleware Registration
        void use(Middleware::MiddlewareFn middleware);

        // Lifecycle
        [[nodiscard]] bool start(uint16_t port);    
        void run();
        void stop() noexcept;
        [[nodiscard]] bool isRunning() const noexcept;

        private:

        // Private Helpers
        void handleConnection(Connection connection);
        void runChain(std::size_t index, HTTP::HttpRequest& request, HTTP::HttpResponse& response) const;
    };

} // namespace FalconHTTP::Core