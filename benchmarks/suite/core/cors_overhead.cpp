// FalconHTTP Cors Middleware Overhead Benchmark Suite
// Measures Cors::operator() alone, isolated from the rest of the
// middleware chain measured in chain_overhead.cpp.
//
// Covers the non-preflight path only (a GET request that sets the
// CORS headers and calls next) - the OPTIONS short-circuit path
// returns earlier and would understate typical per-request cost if
// benchmarked instead.
//
// NOTE: no comparison baseline exists for FalconHTTP (see the suite
// README) - this uses BENCH() with a single lambda, timing FalconHTTP
// alone.
//
// Covers:
// - Cors::operator() on a non-OPTIONS request

#include <support/framework.h>

using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Middleware;

// Measures Cors::operator() on a non-OPTIONS request.
static void bench_cors_overhead() {
    Cors cors("https://example.com");
    NextHandler next = [](HttpRequest&, HttpResponse&) {};

    auto f = [&] {
        HttpRequest request;
        request.setMethod(HttpMethod::Get);

        HttpResponse response;
        cors(request, response, next);
    };

    BENCH("Cors::operator() non-preflight", f);
}

// Executes all Cors overhead benchmark cases.
static void run_benchmarks() {
    bench_cors_overhead();
}

REGISTER_BENCH_SUITE();
