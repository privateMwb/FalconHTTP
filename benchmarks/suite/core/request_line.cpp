// FalconHTTP Request Line Parsing Benchmark Suite
// Measures HttpParser::parse() performance on the smallest realistic
// request shape — a request line plus a handful of typical headers,
// no body.
//
// This is the baseline parsing cost every request pays regardless of
// header or body size - the floor the Core category's other parsing
// benchmarks (header_heavy, body_heavy) build on top of.
//
// NOTE: no comparison baseline exists for FalconHTTP (see the suite
// README) - this runs through BENCH_SOLO() rather than BENCH(), timing
// FalconHTTP alone.
//
// Covers:
// - parse() on a minimal GET request line + Host/User-Agent/Accept headers

#include <support/framework.h>

using namespace FalconHTTP::HTTP;

namespace {
const std::string kRawRequest = "GET /health HTTP/1.1\r\n"
                                "Host: example.com\r\n"
                                "User-Agent: FalconHTTP-Bench/1.0\r\n"
                                "Accept: */*\r\n"
                                "\r\n";
} // namespace

// Measures parse() on a minimal request line + a handful of headers.
static void bench_parse_request_line() {
    auto f = [&] { HttpRequest request = HttpParser::parse(kRawRequest); };

    BENCH("parse() minimal request line", f);
}

// Executes all request-line parsing benchmark cases.
static void run_benchmarks() {
    bench_parse_request_line();
}

REGISTER_BENCH_SUITE();
