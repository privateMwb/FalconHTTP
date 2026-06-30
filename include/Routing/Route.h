#pragma once

#include "HTTP/HttpMethod.h"
#include "RouteHandler.h"

#include <string>

namespace FalconHTTP::Routing {

    class Route {
        public:
        
        // Identity
        HTTP::HttpMethod  method = HTTP::HttpMethod::Unknown;
        std::string       pattern;
        RouteHandler      handler;

        // Constructors
        Route() noexcept = default;
        Route(HTTP::HttpMethod method, std::string pattern, RouteHandler handler);
    };

} // namespace FalconHTTP::Routing