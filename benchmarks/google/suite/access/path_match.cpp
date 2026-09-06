// FalconHTTP PathMatcher Access Benchmark Suite
// Measures PathMatcher::match() performance against a fixed pattern -
// the segment-by-segment comparison every Router::dispatch() call pays
// once per candidate route until a match (or exhaustion) is found.
//
// This is the read/lookup primitive dispatch_hit.cpp and dispatch_miss.cpp
// build on top of - isolating it here shows the matching cost on its own,
// without Router's route-table iteration around it.
//
// NOTE: no comparison baseline exists for FalconHTTP, so this only
// times FalconHTTP alone.
//
// Covers:
// - match() on a pattern with two :param segments against a matching path

#include <FalconHTTP/FalconHTTP.h>
#include <benchmark/benchmark.h>

using namespace FalconHTTP::Routing;

namespace {
const std::string_view kPattern = "/posts/:postId/comments/:commentId";
const std::string_view kPath = "/posts/42/comments/7";
} // namespace

// Measures match() on a pattern with two :param segments.
static void bench_path_match(benchmark::State& state) {
    for (auto _ : state) {
        HashMap<std::string, std::string> params;
        bool matched = PathMatcher::match(kPattern, kPath, params);
        benchmark::DoNotOptimize(matched);
    }
}
BENCHMARK(bench_path_match);
