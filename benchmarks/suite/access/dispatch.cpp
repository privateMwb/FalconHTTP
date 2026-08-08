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
// NOTE: no comparison baseline exists for FalconHTTP (see the suite
// README) - this runs through BENCH_SOLO() rather than BENCH(), timing
// FalconHTTP alone.
//
// Covers:
// - dispatch() on a GET request matching the last of several registered routes
// - dispatch() on a GET request matching none of those routes

#include <support/framework.h>

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
static void bench_dispatch_hit() {
    Router router = buildRouter();

    auto f = [&] {
        HttpRequest request;
        request.setMethod(HttpMethod::Get);
        request.setPath("/posts/42/comments/7");

        HttpResponse response;
        DispatchResult result = router.dispatch(request, response);
        (void)result;
    };

    BENCH("Router::dispatch() matching route", f);
}

// Measures dispatch() on a request matching none of the registered routes.
static void bench_dispatch_miss() {
    Router router = buildRouter();

    auto f = [&] {
        HttpRequest request;
        request.setMethod(HttpMethod::Get);
        request.setPath("/does/not/exist");

        HttpResponse response;
        DispatchResult result = router.dispatch(request, response);
        (void)result;
    };

    BENCH("Router::dispatch() no matching route", f);
}

// Executes both the dispatch-hit and dispatch-miss benchmark cases.
static void run_benchmarks() {
    bench_dispatch_hit();
    std::cout << "\n";

    bench_dispatch_miss();
}

REGISTER_BENCH_SUITE();
