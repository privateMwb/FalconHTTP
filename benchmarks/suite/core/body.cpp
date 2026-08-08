// FalconHTTP Body-Heavy Benchmark Suite
// Measures HttpParser::parse() and HttpSerializer::serialize()
// performance against a large body, minimal headers - the parsing and
// serialization halves of the same request/response shape, side by
// side.
//
// parse() takes the entire remainder after the header block verbatim
// as the body (a single std::string(remaining) construction), and
// serialize() appends that body onto the output buffer in one go -
// both isolate large-body copy cost from the per-header cost measured
// in header.cpp.
//
// NOTE: no comparison baseline exists for FalconHTTP (see the suite
// README) - this uses BENCH() with a single lambda, timing FalconHTTP
// alone.
//
// Covers:
// - parse() on a POST request with a 64 KiB body
// - serialize() on a 200 response with a 64 KiB body

#include <support/framework.h>

using namespace FalconHTTP::HTTP;

namespace {
constexpr std::size_t kBodySize = 64 * 1024;

std::string buildRawRequest() {
    std::string body(kBodySize, 'a');
    std::string raw = "POST /upload HTTP/1.1\r\n"
                      "Host: example.com\r\n"
                      "Content-Length: " +
                      std::to_string(kBodySize) +
                      "\r\n"
                      "\r\n" +
                      body;
    return raw;
}

HttpResponse buildResponse() {
    HttpResponse response(HttpStatus::Ok);
    response.setBody(std::string(kBodySize, 'x'));
    return response;
}

const std::string kRawRequest = buildRawRequest();
} // namespace

// Measures parse() on a request with a 64 KiB body.
static void bench_parse_body_heavy() {
    auto f = [&] { HttpRequest request = HttpParser::parse(kRawRequest); };

    BENCH("parse() 64 KiB body", f);
}

// Measures serialize() on a response with a 64 KiB body.
static void bench_serialize_body_heavy() {
    HttpResponse response = buildResponse();

    auto f = [&] { std::string output = HttpSerializer::serialize(response); };

    BENCH("serialize() 64 KiB body", f);
}

// Executes both the body-heavy parsing and serialization benchmark cases.
static void run_benchmarks() {
    bench_parse_body_heavy();
    std::cout << "\n";

    bench_serialize_body_heavy();
}

REGISTER_BENCH_SUITE();
