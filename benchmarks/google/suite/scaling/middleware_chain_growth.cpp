// FalconHTTP Middleware Chain Growth Benchmark Suite
// Measures how per-request chain execution cost changes as the number
// of registered middleware grows.
//
// Complements chain_overhead.cpp (Core) - that benchmark fixes chain
// length at 3 and repeats the call many times; this one holds
// iteration count fixed per tier and instead grows the chain length
// itself, showing whether per-middleware overhead stays linear.
//
// Server::runChain() (the real chain-execution logic) is private, so
// this drives the same MiddlewareFn/NextHandler contract directly with
// a small local chain runner mirroring runChain()'s recursive pattern -
// the same approach used in chain_overhead.cpp and
// suite/integration/middleware_chain_order.cpp.
//
// NOTE: no comparison baseline exists for FalconHTTP, so this only
// times FalconHTTP alone.
//
// Covers:
// - Running one request through a 1-middleware chain + terminal handler
// - Running one request through a 5-middleware chain + terminal handler
// - Running one request through a 20-middleware chain + terminal handler
// - Running one request through a 50-middleware chain + terminal handler

#include <FalconHTTP/FalconHTTP.h>
#include <benchmark/benchmark.h>

// clang-format off
#include <functional> // std::function
#include <vector>     // std::vector
// clang-format on

using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Middleware;

namespace {

// Mirrors Server::runChain()'s recursive structure - see
// chain_overhead.cpp for the identical implementation and rationale.
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

std::vector<MiddlewareFn> buildChain(int length) {
    std::vector<MiddlewareFn> chain;
    for (int i = 0; i < length; ++i) {
        chain.push_back(
            [](HttpRequest& req, HttpResponse& res, const NextHandler& next) { next(req, res); });
    }
    return chain;
}

const std::vector<MiddlewareFn> kChain1 = buildChain(1);
const std::vector<MiddlewareFn> kChain5 = buildChain(5);
const std::vector<MiddlewareFn> kChain20 = buildChain(20);
const std::vector<MiddlewareFn> kChain50 = buildChain(50);

void runOneRequest(const std::vector<MiddlewareFn>& chain) {
    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/health");

    HttpResponse response;

    runChain(chain, 0, request, response,
             [](HttpRequest&, HttpResponse& res) { res.setStatus(HttpStatus::Ok); });
}

} // namespace

// Measures running one request through a 1-middleware chain.
static void bench_chain_length_1(benchmark::State& state) {
    for (auto _ : state) {
        runOneRequest(kChain1);
    }
}
BENCHMARK(bench_chain_length_1);

// Measures running one request through a 5-middleware chain.
static void bench_chain_length_5(benchmark::State& state) {
    for (auto _ : state) {
        runOneRequest(kChain5);
    }
}
BENCHMARK(bench_chain_length_5);

// Measures running one request through a 20-middleware chain.
static void bench_chain_length_20(benchmark::State& state) {
    for (auto _ : state) {
        runOneRequest(kChain20);
    }
}
BENCHMARK(bench_chain_length_20);

// Measures running one request through a 50-middleware chain.
static void bench_chain_length_50(benchmark::State& state) {
    for (auto _ : state) {
        runOneRequest(kChain50);
    }
}
BENCHMARK(bench_chain_length_50);
