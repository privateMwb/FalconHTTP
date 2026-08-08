// FalconHTTP PathMatcher Access Benchmark Suite
// Measures PathMatcher::match() performance against a fixed pattern -
// the segment-by-segment comparison every Router::dispatch() call pays
// once per candidate route until a match (or exhaustion) is found.
//
// This is the read/lookup primitive dispatch_hit.cpp and dispatch_miss.cpp
// build on top of - isolating it here shows the matching cost on its own,
// without Router's route-table iteration around it.
//
// NOTE: no comparison baseline exists for FalconHTTP (see the suite
// README) - this runs through BENCH_SOLO() rather than BENCH(), timing
// FalconHTTP alone.
//
// Covers:
// - match() on a pattern with two :param segments against a matching path

#include <support/framework.h>

using namespace FalconHTTP::Routing;

namespace {
const std::string_view kPattern = "/posts/:postId/comments/:commentId";
const std::string_view kPath = "/posts/42/comments/7";
} // namespace

// Measures match() on a pattern with two :param segments.
static void bench_path_match() {
    auto f = [&] {
        HashMap<std::string, std::string> params;
        bool matched = PathMatcher::match(kPattern, kPath, params);
        (void)matched;
    };

    BENCH("PathMatcher::match() two params", f);
}

// Executes all path matching benchmark cases.
static void run_benchmarks() {
    bench_path_match();
}

REGISTER_BENCH_SUITE();
