/**
 * @file Router.cpp
 * @brief Router implementation.
 */

// clang-format off
#include <FalconHTTP/Routing/Router.h>      // Router (own header)
#include <FalconHTTP/Routing/PathMatcher.h> // PathMatcher::match
// clang-format on

namespace FalconHTTP::Routing {

// Registration
void Router::get(std::string pattern, RouteHandler handler) {
    addRoute(HTTP::HttpMethod::Get, std::move(pattern), std::move(handler));
}

void Router::post(std::string pattern, RouteHandler handler) {
    addRoute(HTTP::HttpMethod::Post, std::move(pattern), std::move(handler));
}

void Router::put(std::string pattern, RouteHandler handler) {
    addRoute(HTTP::HttpMethod::Put, std::move(pattern), std::move(handler));
}

void Router::del(std::string pattern, RouteHandler handler) {
    addRoute(HTTP::HttpMethod::Delete, std::move(pattern), std::move(handler));
}

// Dispatch
DispatchResult Router::dispatch(HTTP::HttpRequest& request, HTTP::HttpResponse& response) const {
    HashMap<std::string, std::string> params;
    bool pathMatchedOtherMethod = false;

    for (const Route& route : routes) {
        if (route.method != request.method()) {
            HashMap<std::string, std::string> discarded;
            if (PathMatcher::match(route.pattern, request.path(), discarded)) {
                pathMatchedOtherMethod = true;
            }
            continue;
        }

        params.clear();

        if (PathMatcher::match(route.pattern, request.path(), params)) {
            for (const auto& entry : params) {
                request.setPathParam(entry.key, entry.value);
            }

            route.handler(request, response);
            return DispatchResult::Matched;
        }
    }

    return pathMatchedOtherMethod ? DispatchResult::MethodNotAllowed : DispatchResult::NotFound;
}

// Private Helpers
void Router::addRoute(HTTP::HttpMethod method, std::string pattern, RouteHandler handler) {
    routes.push_back(Route(method, std::move(pattern), std::move(handler)));
}

} // namespace FalconHTTP::Routing