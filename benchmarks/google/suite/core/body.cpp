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
// NOTE: no comparison baseline exists for FalconHTTP, so this only
// times FalconHTTP alone.
//
// Covers:
// - parse() on a POST request with a 64 KiB body
// - serialize() on a 200 response with a 64 KiB body

#include <FalconHTTP/FalconHTTP.h>
#include <benchmark/benchmark.h>

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
static void bench_parse_body_heavy(benchmark::State& state) {
    for (auto _ : state) {
        HttpRequest request = HttpParser::parse(kRawRequest);
        benchmark::DoNotOptimize(request);
    }
}
BENCHMARK(bench_parse_body_heavy);

// Measures serialize() on a response with a 64 KiB body.
static void bench_serialize_body_heavy(benchmark::State& state) {
    HttpResponse response = buildResponse();

    for (auto _ : state) {
        std::string output = HttpSerializer::serialize(response);
        benchmark::DoNotOptimize(output);
    }
}
BENCHMARK(bench_serialize_body_heavy);
