// FalconHTTP FileCache Insert Benchmark Suite
// Measures FileCache::put() performance for fresh, non-colliding keys.
//
// Capacity is sized well above the iteration tiers this runs at, so
// no eviction occurs during the benchmark - this isolates pure insert
// cost from LRU-eviction cost (a separate concern, not currently
// broken out into its own benchmark). See cache_hit.cpp (Access) for
// the corresponding read-path cost.
//
// NOTE: no comparison baseline exists for FalconHTTP, so this only
// times FalconHTTP alone.
//
// Covers:
// - put() inserting a fresh key/Entry pair each call

#include <FalconHTTP/FalconHTTP.h>
#include <benchmark/benchmark.h>

using namespace FalconHTTP::FileServer;

namespace {
constexpr std::size_t kCacheCapacity = 2'000'000;
} // namespace

// Measures put() inserting a fresh key each call, no eviction.
static void bench_cache_put(benchmark::State& state) {
    FileCache cache(kCacheCapacity);
    int counter = 0;

    for (auto _ : state) {
        FileCache::Entry entry;
        entry.content = "content";
        entry.contentType = "text/plain";
        cache.put("/file-" + std::to_string(counter), entry);
        ++counter;
    }
}
BENCHMARK(bench_cache_put);
