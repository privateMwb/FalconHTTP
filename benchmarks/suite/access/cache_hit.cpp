// FalconHTTP FileCache Hit Benchmark Suite
// Measures FileCache::get() performance on an entry already present in
// the cache - the LRU lookup and touch cost, without any disk I/O or
// StaticFileServer path resolution around it.
//
// A single entry is put() once outside the timed section; every
// iteration then repeats the same hit, matching the SMALL/MEDIUM/LARGE
// iteration tiers this suite runs at (see the suite README).
//
// NOTE: no comparison baseline exists for FalconHTTP (see the suite
// README) - this runs through BENCH_SOLO() rather than BENCH(), timing
// FalconHTTP alone.
//
// Covers:
// - get() on a path already present in the cache

#include <support/framework.h>

using namespace FalconHTTP::FileServer;

namespace {
const std::string kPath = "/static/index.html";
} // namespace

// Measures get() on an entry the cache already holds.
static void bench_cache_hit() {
    FileCache cache(/*capacity=*/64);
    cache.put(kPath, FileCache::Entry{"<h1>Hello, cache!</h1>", "text/html"});

    auto f = [&] {
        FileCache::Entry outEntry;
        bool found = cache.get(kPath, outEntry);
        (void)found;
    };

    BENCH("FileCache::get() existing entry", f);
}

// Executes all cache-hit benchmark cases.
static void run_benchmarks() {
    bench_cache_hit();
}

REGISTER_BENCH_SUITE();
