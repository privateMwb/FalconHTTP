// FalconHTTP Header-Heavy Benchmark Suite
// Measures HttpParser::parse() and HttpSerializer::serialize()
// performance against a large number of headers, minimal body - the
// parsing and serialization halves of the same request/response shape,
// side by side.
//
// parse() pays the per-header cost once per line via parseHeaderLine(),
// and serialize() pays a mirrored per-header cost in appendHeaders() -
// both isolate that marginal per-header cost from the fixed
// request-line/status-line and body cost measured in body.cpp and
// request_line.cpp.
//
// NOTE: no comparison baseline exists for FalconHTTP (see the suite
// README) - this uses BENCH() with a single lambda, timing FalconHTTP
// alone.
//
// Covers:
// - parse() on a GET request with 50 headers
// - serialize() on a 200 response with 50 headers

#include <support/framework.h>

using namespace FalconHTTP::HTTP;

namespace {
constexpr int kHeaderCount = 50;

std::string buildRawRequest() {
    std::string raw = "GET /health HTTP/1.1\r\nHost: example.com\r\n";
    for (int i = 0; i < kHeaderCount; ++i) {
        raw += "X-Header-" + std::to_string(i) + ": value-" + std::to_string(i) + "\r\n";
    }
    raw += "\r\n";
    return raw;
}

HttpResponse buildResponse() {
    HttpResponse response(HttpStatus::Ok);
    for (int i = 0; i < kHeaderCount; ++i) {
        response.setHeader("X-Header-" + std::to_string(i), "value-" + std::to_string(i));
    }
    response.setBody("ok");
    return response;
}

const std::string kRawRequest = buildRawRequest();
} // namespace

// Measures parse() on a request with 50 headers.
static void bench_parse_header_heavy() {
    auto f = [&] { HttpRequest request = HttpParser::parse(kRawRequest); };

    BENCH("parse() 50 headers", f);
}

// Measures serialize() on a response with 50 headers.
static void bench_serialize_header_heavy() {
    HttpResponse response = buildResponse();

    auto f = [&] { std::string output = HttpSerializer::serialize(response); };

    BENCH("serialize() 50 headers", f);
}

// Executes both the header-heavy parsing and serialization benchmark cases.
static void run_benchmarks() {
    bench_parse_header_heavy();
    std::cout << "\n";

    bench_serialize_header_heavy();
}

REGISTER_BENCH_SUITE();
