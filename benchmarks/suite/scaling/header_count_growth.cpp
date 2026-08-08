// FalconHTTP Header Count Growth Benchmark Suite
// Measures how HttpParser::parse()'s per-call cost changes as the
// number of headers in a request grows.
//
// Complements header_heavy.cpp (Core) - that benchmark fixes header
// count at 50 and repeats the call many times; this one holds
// iteration count fixed per tier and instead grows the header count
// itself, showing whether per-header cost stays linear or degrades as
// the header block gets larger.
//
// NOTE: no comparison baseline exists for FalconHTTP (see the suite
// README) - this uses BENCH() with a single lambda per tier, timing
// FalconHTTP alone.
//
// Covers:
// - parse() on a request with 5 headers
// - parse() on a request with 25 headers
// - parse() on a request with 100 headers
// - parse() on a request with 500 headers

#include <support/framework.h>

using namespace FalconHTTP::HTTP;

namespace {

std::string buildRawRequest(int headerCount) {
    std::string raw = "GET /health HTTP/1.1\r\nHost: example.com\r\n";
    for (int i = 0; i < headerCount; ++i) {
        raw += "X-Header-" + std::to_string(i) + ": value-" + std::to_string(i) + "\r\n";
    }
    raw += "\r\n";
    return raw;
}

const std::string kRaw5 = buildRawRequest(5);
const std::string kRaw25 = buildRawRequest(25);
const std::string kRaw100 = buildRawRequest(100);
const std::string kRaw500 = buildRawRequest(500);

} // namespace

// Measures parse() on a request with 5 headers.
static void bench_parse_5_headers() {
    auto f = [&] { HttpRequest request = HttpParser::parse(kRaw5); };

    BENCH("parse() 5 headers", f);
}

// Measures parse() on a request with 25 headers.
static void bench_parse_25_headers() {
    auto f = [&] { HttpRequest request = HttpParser::parse(kRaw25); };

    BENCH("parse() 25 headers", f);
}

// Measures parse() on a request with 100 headers.
static void bench_parse_100_headers() {
    auto f = [&] { HttpRequest request = HttpParser::parse(kRaw100); };

    BENCH("parse() 100 headers", f);
}

// Measures parse() on a request with 500 headers.
static void bench_parse_500_headers() {
    auto f = [&] { HttpRequest request = HttpParser::parse(kRaw500); };

    BENCH("parse() 500 headers", f);
}

// Executes all header count growth benchmark cases.
static void run_benchmarks() {
    bench_parse_5_headers();
    std::cout << "\n";

    bench_parse_25_headers();
    std::cout << "\n";

    bench_parse_100_headers();
    std::cout << "\n";

    bench_parse_500_headers();
    std::cout << "\n";
}

REGISTER_BENCH_SUITE();
