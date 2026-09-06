// FalconHTTP Header Count Growth Benchmark Suite
// Measures how HttpParser::parse()'s per-call cost changes as the
// number of headers in a request grows.
//
// Complements header.cpp (Core) - that benchmark fixes header
// count at 50 and repeats the call many times; this one holds
// iteration count fixed per tier and instead grows the header count
// itself, showing whether per-header cost stays linear or degrades as
// the header block gets larger.
//
// NOTE: no comparison baseline exists for FalconHTTP, so this only
// times FalconHTTP alone.
//
// Covers:
// - parse() on a request with 5 headers
// - parse() on a request with 25 headers
// - parse() on a request with 100 headers
// - parse() on a request with 500 headers

#include <FalconHTTP/FalconHTTP.h>
#include <benchmark/benchmark.h>

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
static void bench_parse_5_headers(benchmark::State& state) {
    for (auto _ : state) {
        HttpRequest request = HttpParser::parse(kRaw5);
        benchmark::DoNotOptimize(request);
    }
}
BENCHMARK(bench_parse_5_headers);

// Measures parse() on a request with 25 headers.
static void bench_parse_25_headers(benchmark::State& state) {
    for (auto _ : state) {
        HttpRequest request = HttpParser::parse(kRaw25);
        benchmark::DoNotOptimize(request);
    }
}
BENCHMARK(bench_parse_25_headers);

// Measures parse() on a request with 100 headers.
static void bench_parse_100_headers(benchmark::State& state) {
    for (auto _ : state) {
        HttpRequest request = HttpParser::parse(kRaw100);
        benchmark::DoNotOptimize(request);
    }
}
BENCHMARK(bench_parse_100_headers);

// Measures parse() on a request with 500 headers.
static void bench_parse_500_headers(benchmark::State& state) {
    for (auto _ : state) {
        HttpRequest request = HttpParser::parse(kRaw500);
        benchmark::DoNotOptimize(request);
    }
}
BENCHMARK(bench_parse_500_headers);
