// FalconHTTP Router::matchStream() Benchmark Suite
// Measures Router::matchStream() performance on a matching request
// (hit) and on a request matching no registered Stream-kind route
// (miss) - the streaming counterpart to dispatch.cpp in this same
// category, and built the same way: the route table holds a handful
// of stream routes with the matching one placed last, so the hit case
// reflects a realistic partial scan rather than an immediate
// first-try hit.
//
// NOTE: no comparison baseline exists for FalconHTTP (see the suite
// README) - this uses BENCH_SOLO() with a single lambda, timing FalconHTTP
// alone.
//
// Covers:
// - matchStream() on a GET request matching the last of several registered stream routes
// - matchStream() on a GET request matching none of those routes

#include <support/framework.h>

using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Routing;
using namespace FalconHTTP::Streaming;

namespace {

Router buildRouter() {
    Router router;

    router.stream("/logs", [](const HttpRequest&, SseConnection&) {});
    router.stream("/status", [](const HttpRequest&, SseConnection&) {});
    router.stream("/metrics", [](const HttpRequest&, SseConnection&) {});
    router.stream("/rooms/:id/events", [](const HttpRequest&, SseConnection&) {});

    return router;
}

} // namespace

// Measures matchStream() on a request matching the last registered
// stream route.
static void bench_match_stream_hit() {
    Router router = buildRouter();

    auto f = [&] {
        HttpRequest request;
        request.setMethod(HttpMethod::Get);
        request.setPath("/rooms/42/events");

        const StreamHandler* handler = router.matchStream(request);
        (void)handler;
    };

    BENCH_SOLO("Router::matchStream() matching route", f);
}

// Measures matchStream() on a request matching none of the registered
// stream routes.
static void bench_match_stream_miss() {
    Router router = buildRouter();

    auto f = [&] {
        HttpRequest request;
        request.setMethod(HttpMethod::Get);
        request.setPath("/does/not/exist");

        const StreamHandler* handler = router.matchStream(request);
        (void)handler;
    };

    BENCH_SOLO("Router::matchStream() no matching route", f);
}

// Executes both the matchStream-hit and matchStream-miss benchmark cases.
static void run_benchmarks() {
    bench_match_stream_hit();
    std::cout << "\n";

    bench_match_stream_miss();
}

REGISTER_BENCH_SUITE();
