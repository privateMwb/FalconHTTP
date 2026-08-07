// Integrating Logger with real route dispatch.
//
// Demonstrates:
// - Logger wired into a chain that ends in an actual Router::dispatch(),
//   not a placeholder handler
// - The logged line reflects the *final* response, including a status a
//   route handler set, and one Router produced itself (404)
// - Logger's own output format: "[INFO] METHOD path status reason durationMs ms"

#include <support/framework.h>

#include <iostream>

using namespace FalconHTTP;

namespace {
// Same recursive pattern Server::runChain() uses, but the terminal
// step is a real router.dispatch() call instead of a stand-in - this
// is what actually happens inside Server for every request.
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

    Routing::Router router;
    router.get("/health", [](const HTTP::HttpRequest&, HTTP::HttpResponse& response) {
        response.setStatus(HTTP::HttpStatus::Ok);
        response.setBody("healthy");
    });

    Vector<Middleware::MiddlewareFn> chain;
    chain.push_back(Middleware::Logger{});

    // Logger calls next() first, so the line it logs (printed below by
    // Logger itself, to stdout) reflects the response a route handler
    // actually produced - not whatever the response looked like on entry.
    setTitle("Logging a Matched Route");

    HTTP::HttpRequest matchedRequest;
    matchedRequest.setMethod(HTTP::HttpMethod::Get);
    matchedRequest.setPath("/health");

    HTTP::HttpResponse matchedResponse;
    runChain(chain, 0, router, matchedRequest, matchedResponse);

    std::cout << "(log line above came from Logger, not this example)\n\n";

    // The same is true when Router doesn't find a match - runChain() sets
    // 404 before Logger's post-next() code runs, so that's what gets logged.
    setTitle("Logging a 404");

    HTTP::HttpRequest missingRequest;
    missingRequest.setMethod(HTTP::HttpMethod::Get);
    missingRequest.setPath("/does-not-exist");

    HTTP::HttpResponse missingResponse;
    runChain(chain, 0, router, missingRequest, missingResponse);

    std::cout
        << "(log line above shows status 404, even though Logger never touched it directly)\n";
}

REGISTER_EXAMPLE_SUITE();
