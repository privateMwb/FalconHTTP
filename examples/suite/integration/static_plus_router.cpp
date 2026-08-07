// Combining API routes with static file serving.
//
// Demonstrates:
// - Router has no wildcard/catch-all pattern, so "serve everything under
//   /public/*" can't be expressed as a single route
// - The alternative: a middleware that tries StaticFileServer::serve()
//   first and only calls next() (continuing to API routing) on a miss
// - API routes still winning when a path collides with both a static
//   file and a registered route

#include <support/framework.h>

#include <filesystem>
#include <fstream>
#include <iostream>

using namespace FalconHTTP;

namespace {
void runChain(const Vector<Middleware::MiddlewareFn>& middleware, std::size_t index,
              const Routing::Router& router, HTTP::HttpRequest& request,
              HTTP::HttpResponse& response) {
    if (index >= middleware.size()) {
        Routing::DispatchResult result = router.dispatch(request, response);

        if (result == Routing::DispatchResult::NotFound) {
            response.setStatus(HTTP::HttpStatus::NotFound);
            response.setBody("Not Found");
        } else if (result == Routing::DispatchResult::MethodNotAllowed) {
            response.setStatus(HTTP::HttpStatus::MethodNotAllowed);
            response.setBody("Method Not Allowed");
        }
        return;
    }

    Middleware::NextHandler next(
        [&middleware, index, &router](HTTP::HttpRequest& request, HTTP::HttpResponse& response) {
            runChain(middleware, index + 1, router, request, response);
        });

    middleware[index](request, response, next);
}
} // namespace

static void run_examples() {

    setTitle("Setup");

    std::filesystem::path root =
        std::filesystem::temp_directory_path() / "falconhttp_combined_example";
    std::filesystem::create_directories(root);
    std::ofstream(root / "about.html") << "<h1>About</h1>";

    FileServer::StaticFileServer staticFiles(root.string(), /*cacheCapacity=*/32);

    Routing::Router router;
    router.get("/api/status", [](const HTTP::HttpRequest&, HTTP::HttpResponse& response) {
        response.setStatus(HTTP::HttpStatus::Ok);
        response.setBody("{\"status\":\"ok\"}");
    });

    // This middleware runs before dispatch: it treats the request path as
    // a static file path first. On a hit, it sets the response and does
    // NOT call next() - dispatch never runs for that request. On a miss,
    // it calls next() and API routing gets a chance instead.
    Middleware::MiddlewareFn staticFallback = [&staticFiles](HTTP::HttpRequest& request,
                                                             HTTP::HttpResponse& response,
                                                             const Middleware::NextHandler& next) {
        if (staticFiles.serve(request.path(), response)) {
            return;
        }
        next(request, response);
    };

    Vector<Middleware::MiddlewareFn> chain;
    chain.push_back(staticFallback);

    std::cout << "static root : " << root.string() << "\n";
    std::cout << "routes      : GET /api/status\n\n";

    // A path that resolves to a real file is served without ever reaching
    // the router.
    setTitle("A Static File Hit");

    HTTP::HttpRequest staticRequest;
    staticRequest.setMethod(HTTP::HttpMethod::Get);
    staticRequest.setPath("/about.html");

    HTTP::HttpResponse staticResponse;
    runChain(chain, 0, router, staticRequest, staticResponse);

    std::cout << "status : " << static_cast<int>(staticResponse.status()) << "\n";
    std::cout << "body   : " << staticResponse.body() << "\n\n";

    // A path with no matching file falls through to the router instead.
    setTitle("Falling Through to the Router");

    HTTP::HttpRequest apiRequest;
    apiRequest.setMethod(HTTP::HttpMethod::Get);
    apiRequest.setPath("/api/status");

    HTTP::HttpResponse apiResponse;
    runChain(chain, 0, router, apiRequest, apiResponse);

    std::cout << "status : " << static_cast<int>(apiResponse.status()) << "\n";
    std::cout << "body   : " << apiResponse.body() << "\n\n";

    // A path matching neither a file nor a route still ends up 404,
    // exactly as it would with the router alone.
    setTitle("Matching Neither");

    HTTP::HttpRequest missingRequest;
    missingRequest.setMethod(HTTP::HttpMethod::Get);
    missingRequest.setPath("/nothing-here");

    HTTP::HttpResponse missingResponse;
    runChain(chain, 0, router, missingRequest, missingResponse);

    std::cout << "status : " << static_cast<int>(missingResponse.status()) << "\n";

    std::filesystem::remove_all(root);
}

REGISTER_EXAMPLE_SUITE();
