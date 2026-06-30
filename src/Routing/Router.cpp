#include "Routing/Router.h"
#include "Routing/PathMatcher.h"

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
    bool Router::dispatch(HTTP::HttpRequest& request, HTTP::HttpResponse& response) const {
        HashMap<std::string, std::string> params;

        for (const Route& route : routes) {
            if (route.method != request.method()) {
                continue;
            }

            params.clear();

            if (PathMatcher::match(route.pattern, request.path(), params)) {
                for (const auto& entry : params) {
                    request.setPathParam(entry.key, entry.value);
                }

                route.handler(request, response);
                return true;
            }
        }

        return false;
    }

    // Private Helpers
    void Router::addRoute(HTTP::HttpMethod method, std::string pattern, RouteHandler handler) {
        routes.push_back(Route(method, std::move(pattern), std::move(handler)));
    }

} // namespace FalconHTTP::Routing