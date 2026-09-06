// Middleware chain ordering integration test suite.
//
// Coverage:
// - Registered middleware runs in the onion model: each middleware
//   runs code before calling `next`, then (optionally) after `next`
//   returns, wrapping around the final handler
// - A middleware that doesn't call `next` short-circuits the chain -
//   downstream middleware and the handler never run
//
// NOTE: Server::runChain() (the real chain-execution logic) is
// private, so this suite drives the same MiddlewareFn/NextHandler
// contract directly with a small local chain runner mirroring
// runChain()'s recursive pattern, rather than going through a live
// Server. This tests middleware composability correctly, but is not a
// substitute for exercising Server::runChain() itself.

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

// clang-format off
#include <functional> // std::function (local runNext helper)
// clang-format on

using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Middleware;

namespace {

// Mirrors Server::runChain()'s recursive structure: invokes
// chain[index], passing a NextHandler that recurses to index + 1.
// At index == chain.size(), invokes the terminal handler.
void runChain(const std::vector<MiddlewareFn>& chain, std::size_t index, HttpRequest& request,
              HttpResponse& response,
              const std::function<void(HttpRequest&, HttpResponse&)>& terminal) {
    if (index >= chain.size()) {
        terminal(request, response);
        return;
    }

    chain[index](request, response,
                 [&chain, index, &terminal](HttpRequest& req, HttpResponse& res) {
                     runChain(chain, index + 1, req, res, terminal);
                 });
}

} // namespace

// Verifies middleware runs in registration order around the handler,
// by having each middleware append a marker before and after `next`.
TEST(MiddlewareChainOrder, MiddlewareWrapsHandlerInOnionOrder) {
    std::string trace;

    std::vector<MiddlewareFn> chain;
    chain.push_back([&trace](HttpRequest& req, HttpResponse& res, const NextHandler& next) {
        trace += "A-before,";
        next(req, res);
        trace += "A-after,";
    });
    chain.push_back([&trace](HttpRequest& req, HttpResponse& res, const NextHandler& next) {
        trace += "B-before,";
        next(req, res);
        trace += "B-after,";
    });

    HttpRequest request;
    HttpResponse response;

    runChain(chain, 0, request, response,
             [&trace](HttpRequest&, HttpResponse&) { trace += "handler,"; });

    EXPECT_EQ(trace, "A-before,B-before,handler,B-after,A-after,");
}

// Verifies a middleware that never calls `next` short-circuits the
// chain - neither downstream middleware nor the handler run.
TEST(MiddlewareChainOrder, MiddlewareNotCallingNextShortCircuits) {
    bool secondMiddlewareRan = false;
    bool handlerRan = false;

    std::vector<MiddlewareFn> chain;
    chain.push_back([](HttpRequest&, HttpResponse& res, const NextHandler&) {
        res.setStatus(HttpStatus::Unauthorized);
        // deliberately does not call next()
    });
    chain.push_back(
        [&secondMiddlewareRan](HttpRequest& req, HttpResponse& res, const NextHandler& next) {
            secondMiddlewareRan = true;
            next(req, res);
        });

    HttpRequest request;
    HttpResponse response;

    runChain(chain, 0, request, response,
             [&handlerRan](HttpRequest&, HttpResponse&) { handlerRan = true; });

    EXPECT_FALSE(secondMiddlewareRan);
    EXPECT_FALSE(handlerRan);
    EXPECT_EQ(response.status(), HttpStatus::Unauthorized);
}
