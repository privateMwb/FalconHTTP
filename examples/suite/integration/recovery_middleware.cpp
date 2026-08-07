// Integrating Recovery with real route handlers.
//
// Demonstrates:
// - Recovery wired in front of a Router whose handler throws
// - The exception never reaching the caller - it's replaced by a clean 500
// - Recovery must be registered before (outside) anything that might throw;
//   demonstrating what happens when it's registered too late to help

#include <support/framework.h>

#include <iostream>
#include <stdexcept>

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

    Routing::Router router;
    router.get("/risky", [](const HTTP::HttpRequest&, HTTP::HttpResponse&) {
        throw std::runtime_error("downstream dependency unavailable");
    });

    // With Recovery registered first, its try/catch wraps every middleware
    // and the route handler after it - the throw above never escapes.
    setTitle("Recovery Registered First");

    Vector<Middleware::MiddlewareFn> protectedChain;
    protectedChain.push_back(Middleware::Recovery{});

    HTTP::HttpRequest request;
    request.setMethod(HTTP::HttpMethod::Get);
    request.setPath("/risky");

    HTTP::HttpResponse response;
    runChain(protectedChain, 0, router, request, response);

    std::cout << "status : " << static_cast<int>(response.status()) << "\n";
    std::cout << "body   : " << response.body() << "\n";
    std::cout << "(error line above came from Recovery, written to stderr)\n\n";

    // Middleware registered *after* the site of an exception is never
    // reached - runChain() unwinds straight past it into the nearest
    // enclosing try/catch, which here is nothing, so this chain has no
    // Recovery at all and the exception would propagate to the caller.
    // Server itself has a second, server-level catch (see Server.h) as a
    // last resort, but relying on that means every route shares one
    // generic 500 with no per-route control.
    setTitle("No Recovery Registered");

    Vector<Middleware::MiddlewareFn> unprotectedChain; // deliberately empty

    HTTP::HttpRequest unprotectedRequest;
    unprotectedRequest.setMethod(HTTP::HttpMethod::Get);
    unprotectedRequest.setPath("/risky");

    HTTP::HttpResponse unprotectedResponse;

    try {
        runChain(unprotectedChain, 0, router, unprotectedRequest, unprotectedResponse);
        std::cout << "unreachable - the handler always throws\n";
    } catch (const std::exception& e) {
        std::cout << "exception escaped the chain: " << e.what() << "\n";
    }
}

REGISTER_EXAMPLE_SUITE();
