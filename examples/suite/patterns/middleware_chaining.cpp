// The onion-model middleware chain.
//
// Demonstrates:
// - Registration order vs. execution order (code before next() runs
//   top-down, code after next() runs bottom-up)
// - Building and running a chain the same way Server::runChain() does
// - A middleware short-circuiting the chain by not calling next()

#include <support/framework.h>

#include <iostream>

using namespace FalconHTTP;

namespace {
// Mirrors Server::runChain(): invokes middleware_[index], passing a
// NextHandler that recurses to index + 1. At the end of the chain,
// it stands in for "the route handler" instead of a real Router.
void runChain(const Vector<Middleware::MiddlewareFn>& middleware, std::size_t index,
              HTTP::HttpRequest& request, HTTP::HttpResponse& response) {
    if (index >= middleware.size()) {
        std::cout << "  -> route handler\n";
        response.setStatus(HTTP::HttpStatus::Ok);
        response.setBody("handled");
        return;
    }

    Middleware::NextHandler next(
        [&middleware, index](HTTP::HttpRequest& request, HTTP::HttpResponse& response) {
            runChain(middleware, index + 1, request, response);
        });

    middleware[index](request, response, next);
}
} // namespace

static void run_examples() {

    // Each middleware runs code before calling next(), then (if it calls
    // next()) resumes after next() returns - so registration order A, B, C
    // produces execution order: A-before, B-before, C-before, C-after,
    // B-after, A-after.
    setTitle("Execution Order");

    Vector<Middleware::MiddlewareFn> chain;

    chain.push_back([](HTTP::HttpRequest& request, HTTP::HttpResponse& response,
                       const Middleware::NextHandler& next) {
        std::cout << "A: before\n";
        next(request, response);
        std::cout << "A: after\n";
    });

    chain.push_back([](HTTP::HttpRequest& request, HTTP::HttpResponse& response,
                       const Middleware::NextHandler& next) {
        std::cout << "B: before\n";
        next(request, response);
        std::cout << "B: after\n";
    });

    HTTP::HttpRequest request;
    HTTP::HttpResponse response;
    runChain(chain, 0, request, response);

    std::cout << "\nfinal status : " << static_cast<int>(response.status()) << "\n\n";

    // A middleware that doesn't call next() stops the chain there - nothing
    // registered after it runs, and neither does the route handler. This is
    // exactly how Cors handles OPTIONS preflight requests.
    setTitle("Short-Circuiting");

    Vector<Middleware::MiddlewareFn> guardedChain;

    guardedChain.push_back(
        [](HTTP::HttpRequest&, HTTP::HttpResponse& response, const Middleware::NextHandler&) {
            std::cout << "auth check: rejecting\n";
            response.setStatus(HTTP::HttpStatus::Unauthorized);
            response.setBody("Unauthorized");
            // next() intentionally not called.
        });

    guardedChain.push_back([](HTTP::HttpRequest& request, HTTP::HttpResponse& response,
                              const Middleware::NextHandler& next) {
        std::cout << "this middleware never runs\n";
        next(request, response);
    });

    HTTP::HttpRequest guardedRequest;
    HTTP::HttpResponse guardedResponse;
    runChain(guardedChain, 0, guardedRequest, guardedResponse);

    std::cout << "\nfinal status : " << static_cast<int>(guardedResponse.status()) << "\n";
}

REGISTER_EXAMPLE_SUITE();
