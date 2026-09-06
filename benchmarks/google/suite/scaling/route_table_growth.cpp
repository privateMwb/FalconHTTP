// FalconHTTP Route Table Growth Benchmark Suite
// Measures how Router::dispatch()'s per-call cost changes as the
// number of registered routes grows.
//
// Router::dispatch() is a linear scan over routes (first-match-wins),
// not a trie/radix structure - this benchmark exists specifically to
// put a real number on that O(n) cost, rather than leaving it as a
// theoretical concern. Each tier dispatches to the LAST registered
// route, forcing a full scan of every route before it - the worst
// case for a given table size, and the case that best reveals the
// linear-scan cost as the table grows.
//
// NOTE: no comparison baseline exists for FalconHTTP, so this only
// times FalconHTTP alone. Each tier here is a distinct benchmark
// function with its own fixed route-table size - the growth is in the
// table itself, not in the iteration count.
//
// Covers:
// - dispatch() to the last route in a 10-route table
// - dispatch() to the last route in a 100-route table
// - dispatch() to the last route in a 500-route table
// - dispatch() to the last route in a 2000-route table

#include <FalconHTTP/FalconHTTP.h>
#include <benchmark/benchmark.h>

using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Routing;

namespace {

Router buildRouterWithRoutes(int routeCount) {
    Router router;
    for (int i = 0; i < routeCount; ++i) {
        router.get("/route-" + std::to_string(i), [](const HttpRequest&, HttpResponse&) {});
    }
    return router;
}

Router router10 = buildRouterWithRoutes(10);
Router router100 = buildRouterWithRoutes(100);
Router router500 = buildRouterWithRoutes(500);
Router router2000 = buildRouterWithRoutes(2000);

} // namespace

// Measures dispatch() to the last route in a 10-route table.
static void bench_dispatch_10_routes(benchmark::State& state) {
    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/route-9");

    for (auto _ : state) {
        HttpResponse response;
        DispatchResult result = router10.dispatch(request, response);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(bench_dispatch_10_routes);

// Measures dispatch() to the last route in a 100-route table.
static void bench_dispatch_100_routes(benchmark::State& state) {
    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/route-99");

    for (auto _ : state) {
        HttpResponse response;
        DispatchResult result = router100.dispatch(request, response);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(bench_dispatch_100_routes);

// Measures dispatch() to the last route in a 500-route table.
static void bench_dispatch_500_routes(benchmark::State& state) {
    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/route-499");

    for (auto _ : state) {
        HttpResponse response;
        DispatchResult result = router500.dispatch(request, response);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(bench_dispatch_500_routes);

// Measures dispatch() to the last route in a 2000-route table.
static void bench_dispatch_2000_routes(benchmark::State& state) {
    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/route-1999");

    for (auto _ : state) {
        HttpResponse response;
        DispatchResult result = router2000.dispatch(request, response);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(bench_dispatch_2000_routes);
