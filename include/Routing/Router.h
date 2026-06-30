#pragma once

#include "Route.h"
#include "HTTP/HttpRequest.h"
#include "HTTP/HttpResponse.h"
#include "Vector.h"

namespace FalconHTTP::Routing {
    class Router {
        public:

        // Identity 
        VectorPro::Vector<Route> routes;

        // Constructors
        Router() noexcept = default;

        // Registration
        void get(std::string pattern, RouteHandler handler);
        void post(std::string pattern, RouteHandler handler);
        void put(std::string pattern, RouteHandler handler);
        void del(std::string pattern, RouteHandler handler);

        // Dispatch
        [[nodiscard]] bool dispatch(HTTP::HttpRequest& request, HTTP::HttpResponse& response) const;

        private:

        // Private Helpers
        void addRoute(HTTP::HttpMethod method, std::string pattern, RouteHandler handler);
    };

} // namespace FalconHTTP::Routing

