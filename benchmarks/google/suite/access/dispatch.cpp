// FalconHTTP Router Dispatch Benchmark Suite
// Measures Router::dispatch() performance on a matching request (hit)
// and on a request matching no registered route (miss), against the
// same route table - directly comparable side by side, since a miss
// must exhaust every candidate that a hit can stop partway through.
//
// The route table holds a handful of routes with the matching route
// placed last, so the hit case reflects a realistic partial scan
// through PathMatcher::match() calls rather than an immediate first-try
// hit.
//
// NOTE: no comparison baseline exists for FalconHTTP, so this only
// times FalconHTTP alone.
//
// Covers:
// - dispatch() on a GET request matching the last of several registered routes
// - dispatch() on a GET request matching none of those routes

#include <FalconHTTP/FalconHTTP.h>
#include <benchmark/benchmark.h>

using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Routing;

namespace {

Router buildRouter() {
    Router router;

    router.get("/", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
    });
    router.get("/status", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
    });
    router.post("/posts", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Created);
    });
    router.get("/posts/:postId/comments/:commentId", [](const HttpRequest& request,
                                                        HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
        response.setBody(request.pathParam("postId") + "/" + request.pathParam("commentId"));
    });

    return router;
}

} // namespace

// Measures dispatch() on a request matching the last registered route.
static void bench_dispatch_hit(benchmark::State& state) {
    Router router = buildRouter();

    for (auto _ : state) {
        HttpRequest request;
        request.setMethod(HttpMethod::Get);
        request.setPath("/posts/42/comments/7");

        HttpResponse response;
        DispatchResult result = router.dispatch(request, response);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(bench_dispatch_hit);

// Measures dispatch() on a request matching none of the registered routes.
static void bench_dispatch_miss(benchmark::State& state) {
    Router router = buildRouter();

    for (auto _ : state) {
        HttpRequest request;
        request.setMethod(HttpMethod::Get);
        request.setPath("/does/not/exist");

        HttpResponse response;
        DispatchResult result = router.dispatch(request, response);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(bench_dispatch_miss);
