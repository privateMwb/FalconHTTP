// FalconHTTP Cors Middleware Overhead Benchmark Suite
// Measures Cors::operator() alone, isolated from the rest of the
// middleware chain measured in chain_overhead.cpp.
//
// Covers the non-preflight path only (a GET request that sets the
// CORS headers and calls next) - the OPTIONS short-circuit path
// returns earlier and would understate typical per-request cost if
// benchmarked instead.
//
// NOTE: no comparison baseline exists for FalconHTTP, so this only
// times FalconHTTP alone.
//
// Covers:
// - Cors::operator() on a non-OPTIONS request

#include <FalconHTTP/FalconHTTP.h>
#include <benchmark/benchmark.h>

using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Middleware;

// Measures Cors::operator() on a non-OPTIONS request.
static void bench_cors_overhead(benchmark::State& state) {
    Cors cors("https://example.com");
    NextHandler next = [](HttpRequest&, HttpResponse&) {};

    for (auto _ : state) {
        HttpRequest request;
        request.setMethod(HttpMethod::Get);

        HttpResponse response;
        cors(request, response, next);
        benchmark::DoNotOptimize(response);
    }
}
BENCHMARK(bench_cors_overhead);
