// FalconHTTP FileCache Insert Benchmark Suite
// Measures FileCache::put() performance for fresh, non-colliding keys.
//
// Capacity is sized well above the iteration tiers this runs at, so
// no eviction occurs during the benchmark - this isolates pure insert
// cost from LRU-eviction cost (a separate concern, not currently
// broken out into its own benchmark). See cache_hit.cpp (Access) for
// the corresponding read-path cost.
//
// NOTE: no comparison baseline exists for FalconHTTP (see the suite
// README) - this uses BENCH() with a single lambda, timing FalconHTTP
// alone.
//
// Covers:
// - put() inserting a fresh key/Entry pair each call

#include <support/framework.h>

using namespace FalconHTTP::FileServer;

namespace {
constexpr std::size_t kCacheCapacity = 2'000'000;
} // namespace

// Measures put() inserting a fresh key each call, no eviction.
static void bench_cache_put() {
    FileCache cache(kCacheCapacity);
    int counter = 0;

    auto f = [&] {
        FileCache::Entry entry;
        entry.content = "content";
        entry.contentType = "text/plain";
        cache.put("/file-" + std::to_string(counter), entry);
        ++counter;
    };

    BENCH("FileCache::put() fresh key", f);
}

// Executes all FileCache insert benchmark cases.
static void run_benchmarks() {
    bench_cache_put();
}

REGISTER_BENCH_SUITE();
