// FalconHTTP HTTP Method Conversion Benchmark Suite
// Measures methodFromString() and methodToString() performance.
//
// methodFromString() runs once per parsed request (see
// HttpParser::parseRequestLine()) - a simple, frequently-called
// string comparison chain worth a floor number for.
//
// NOTE: no comparison baseline exists for FalconHTTP, so this only
// times FalconHTTP alone.
//
// Covers:
// - methodFromString() on GET (checked first, best case)
// - methodFromString() on OPTIONS (checked last, worst case)
// - methodToString() on HttpMethod::Get

#include <FalconHTTP/FalconHTTP.h>
#include <benchmark/benchmark.h>

using namespace FalconHTTP::HTTP;

// Measures methodFromString() on GET - the first method checked,
// so this is the best-case comparison chain cost.
static void method_from_string_case_best(benchmark::State& state) {
    for (auto _ : state) {
        HttpMethod method = methodFromString("GET");
        benchmark::DoNotOptimize(method);
    }
}
BENCHMARK(method_from_string_case_best);

// Measures methodFromString() on OPTIONS - the last method checked,
// so this is the worst-case comparison chain cost.
static void method_from_string_case_worst(benchmark::State& state) {
    for (auto _ : state) {
        HttpMethod method = methodFromString("OPTIONS");
        benchmark::DoNotOptimize(method);
    }
}
BENCHMARK(method_from_string_case_worst);

// Measures methodToString() on HttpMethod::Get.
static void method_to_string(benchmark::State& state) {
    for (auto _ : state) {
        std::string_view text = methodToString(HttpMethod::Get);
        benchmark::DoNotOptimize(text);
    }
}
BENCHMARK(method_to_string);
