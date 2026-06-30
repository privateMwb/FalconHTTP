#pragma once

#include "Middleware.h"

#include <string>

namespace FalconHTTP::Middleware {
    
    class Cors {
        private:

        // Storage
        std::string allowedOrigin_;

        public:

        // Constructors
        Cors() noexcept;
        explicit Cors(std::string allowedOrigin);

        // Core API
        void operator()(HTTP::HttpRequest& request, HTTP::HttpResponse& response, const NextHandler& next) const;
    };

} // namespace FalconHTTP::Middleware