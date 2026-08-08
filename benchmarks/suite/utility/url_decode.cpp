// FalconHTTP URL Decoding Benchmark Suite
// Measures UrlDecoder::decode() performance on a typical
// percent-encoded query string value.
//
// This runs once per query parameter on every parsed request (see
// HttpParser::parsePath()), so its per-call cost is paid frequently
// even though each individual call is cheap.
//
// NOTE: no comparison baseline exists for FalconHTTP (see the suite
// README) - this uses BENCH() with a single lambda, timing FalconHTTP
// alone.
//
// Covers:
// - decode() on a string mixing plain text, %XX sequences, and '+'

#include <support/framework.h>

using namespace FalconHTTP::Utility;

namespace {
constexpr std::string_view kEncoded = "hello%20world%21+this+is+a%2Ftest%3Fquery%3Dvalue";
} // namespace

// Measures decode() on a mixed plain/percent-encoded/'+' string.
static void bench_url_decode() {
    auto f = [&] { std::string decoded = UrlDecoder::decode(kEncoded); };

    BENCH("UrlDecoder::decode() mixed encoding", f);
}

// Executes all URL decoding benchmark cases.
static void run_benchmarks() {
    bench_url_decode();
}

REGISTER_BENCH_SUITE();
