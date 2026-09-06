// FalconHTTP MIME Type Lookup Benchmark Suite
// Measures mimeTypeFromExtension() performance for a known extension
// and an unknown one falling back to the default.
//
// Called once per StaticFileServer::serve() call, on both the cache
// hit and cache miss paths - cheap per call, but worth a floor number
// given how frequently static file serving invokes it.
//
// NOTE: no comparison baseline exists for FalconHTTP, so this only
// times FalconHTTP alone.
//
// Covers:
// - mimeTypeFromExtension() on a known extension (".html")
// - mimeTypeFromExtension() on an unknown extension (falls back to
//   application/octet-stream)

#include <FalconHTTP/FalconHTTP.h>
#include <benchmark/benchmark.h>

using namespace FalconHTTP::HTTP;

// Measures the lookup for a known, mapped extension.
static void bench_mime_lookup_known(benchmark::State& state) {
    for (auto _ : state) {
        std::string_view type = mimeTypeFromExtension(".html");
        benchmark::DoNotOptimize(type);
    }
}
BENCHMARK(bench_mime_lookup_known);

// Measures the lookup for an extension not in the built-in table,
// which falls all the way through to the default return.
static void bench_mime_lookup_unknown(benchmark::State& state) {
    for (auto _ : state) {
        std::string_view type = mimeTypeFromExtension(".unknownext");
        benchmark::DoNotOptimize(type);
    }
}
BENCHMARK(bench_mime_lookup_unknown);
