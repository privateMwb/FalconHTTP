// FalconHTTP FileCache Hit Benchmark Suite
// Measures FileCache::get() performance on an entry already present in
// the cache - the LRU lookup and touch cost, without any disk I/O or
// StaticFileServer path resolution around it.
//
// A single entry is put() once outside the timed section; every
// iteration then repeats the same hit.
//
// NOTE: no comparison baseline exists for FalconHTTP, so this only
// times FalconHTTP alone.
//
// Covers:
// - get() on a path already present in the cache

#include <FalconHTTP/FalconHTTP.h>
#include <benchmark/benchmark.h>

using namespace FalconHTTP::FileServer;

namespace {
const std::string kPath = "/static/index.html";
} // namespace

// Measures get() on an entry the cache already holds.
static void bench_cache_hit(benchmark::State& state) {
    FileCache cache(/*capacity=*/64);
    cache.put(kPath, FileCache::Entry{"<h1>Hello, cache!</h1>", "text/html"});

    for (auto _ : state) {
        FileCache::Entry outEntry;
        bool found = cache.get(kPath, outEntry);
        benchmark::DoNotOptimize(found);
    }
}
BENCHMARK(bench_cache_hit);
