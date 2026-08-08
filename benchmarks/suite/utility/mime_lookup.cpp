// FalconHTTP MIME Type Lookup Benchmark Suite
// Measures mimeTypeFromExtension() performance for a known extension
// and an unknown one falling back to the default.
//
// Called once per StaticFileServer::serve() call, on both the cache
// hit and cache miss paths - cheap per call, but worth a floor number
// given how frequently static file serving invokes it.
//
// NOTE: no comparison baseline exists for FalconHTTP (see the suite
// README) - this uses BENCH() with a single lambda, timing FalconHTTP
// alone.
//
// Covers:
// - mimeTypeFromExtension() on a known extension (".html")
// - mimeTypeFromExtension() on an unknown extension (falls back to
//   application/octet-stream)

#include <support/framework.h>

using namespace FalconHTTP::HTTP;

// Measures the lookup for a known, mapped extension.
static void bench_mime_lookup_known() {
    auto f = [&] {
        std::string_view type = mimeTypeFromExtension(".html");
        (void)type;
    };

    BENCH("mimeTypeFromExtension() known extension", f);
}

// Measures the lookup for an extension not in the built-in table,
// which falls all the way through to the default return.
static void bench_mime_lookup_unknown() {
    auto f = [&] {
        std::string_view type = mimeTypeFromExtension(".unknownext");
        (void)type;
    };

    BENCH("mimeTypeFromExtension() unknown extension", f);
}

// Executes all MIME type lookup benchmark cases.
static void run_benchmarks() {
    bench_mime_lookup_known();
    std::cout << "\n";

    bench_mime_lookup_unknown();
}

REGISTER_BENCH_SUITE();
