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
// NOTE: no comparison baseline exists for FalconHTTP (see the suite
// README) - this uses BENCH() with a single lambda per tier, timing
// FalconHTTP alone. Unlike Core/Access benchmarks, each tier here is a
// distinct BENCH() call with its own fixed route-table size - the
// growth is in the table itself, not in the SMALL/MEDIUM/LARGE
// iteration count each BENCH() call already runs internally.
//
// Covers:
// - dispatch() to the last route in a 10-route table
// - dispatch() to the last route in a 100-route table
// - dispatch() to the last route in a 500-route table
// - dispatch() to the last route in a 2000-route table

#include <support/framework.h>

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
static void bench_dispatch_10_routes() {
    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/route-9");

    auto f = [&] {
        HttpResponse response;
        (void)router10.dispatch(request, response);
    };

    BENCH("dispatch() last of 10 routes", f);
}

// Measures dispatch() to the last route in a 100-route table.
static void bench_dispatch_100_routes() {
    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/route-99");

    auto f = [&] {
        HttpResponse response;
        (void)router100.dispatch(request, response);
    };

    BENCH("dispatch() last of 100 routes", f);
}

// Measures dispatch() to the last route in a 500-route table.
static void bench_dispatch_500_routes() {
    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/route-499");

    auto f = [&] {
        HttpResponse response;
        (void)router500.dispatch(request, response);
    };

    BENCH("dispatch() last of 500 routes", f);
}

// Measures dispatch() to the last route in a 2000-route table.
static void bench_dispatch_2000_routes() {
    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/route-1999");

    auto f = [&] {
        HttpResponse response;
        (void)router2000.dispatch(request, response);
    };

    BENCH("dispatch() last of 2000 routes", f);
}

// Executes all route table growth benchmark cases.
static void run_benchmarks() {
    bench_dispatch_10_routes();
    std::cout << "\n";

    bench_dispatch_100_routes();
    std::cout << "\n";

    bench_dispatch_500_routes();
    std::cout << "\n";

    bench_dispatch_2000_routes();
}

REGISTER_BENCH_SUITE();
