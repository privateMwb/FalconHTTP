// FalconHTTP Middleware Chain Overhead Benchmark Suite
// Measures the per-request cost of running a fixed-size middleware
// chain (3 middleware) around a terminal handler.
//
// Server::runChain() (the real chain-execution logic) is private, so
// this drives the same MiddlewareFn/NextHandler contract directly
// with a small local chain runner mirroring runChain()'s recursive
// pattern - the same approach used in
// suite/integration/middleware_chain_order.cpp. See
// middleware_chain_growth.cpp (Scaling) for how this cost changes as
// chain length grows; this benchmark fixes the length at 3 and
// measures steady-state per-request cost.
//
// NOTE: no comparison baseline exists for FalconHTTP (see the suite
// README) - this uses BENCH() with a single lambda, timing FalconHTTP
// alone.
//
// Covers:
// - Running one request through a 3-middleware chain + terminal handler

#include <support/framework.h>

// clang-format off
#include <functional> // std::function
#include <vector>     // std::vector
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

std::vector<MiddlewareFn> buildChain() {
    std::vector<MiddlewareFn> chain;
    for (int i = 0; i < 3; ++i) {
        chain.push_back(
            [](HttpRequest& req, HttpResponse& res, const NextHandler& next) { next(req, res); });
    }
    return chain;
}

} // namespace

// Measures running one request through a 3-middleware chain and a
// terminal handler.
static void bench_chain_overhead() {
    std::vector<MiddlewareFn> chain = buildChain();

    auto f = [&] {
        HttpRequest request;
        request.setMethod(HttpMethod::Get);
        request.setPath("/health");

        HttpResponse response;

        runChain(chain, 0, request, response,
                 [](HttpRequest&, HttpResponse& res) { res.setStatus(HttpStatus::Ok); });
    };

    BENCH("3-middleware chain + handler", f);
}

// Executes all middleware chain overhead benchmark cases.
static void run_benchmarks() {
    bench_chain_overhead();
}

REGISTER_BENCH_SUITE();
