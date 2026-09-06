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
static void bench_method_from_string_best_case(benchmark::State& state) {
    for (auto _ : state) {
        HttpMethod method = methodFromString("GET");
        benchmark::DoNotOptimize(method);
    }
}
BENCHMARK(bench_method_from_string_best_case);

// Measures methodFromString() on OPTIONS - the last method checked,
// so this is the worst-case comparison chain cost.
static void bench_method_from_string_worst_case(benchmark::State& state) {
    for (auto _ : state) {
        HttpMethod method = methodFromString("OPTIONS");
        benchmark::DoNotOptimize(method);
    }
}
BENCHMARK(bench_method_from_string_worst_case);

// Measures methodToString() on HttpMethod::Get.
static void bench_method_to_string(benchmark::State& state) {
    for (auto _ : state) {
        std::string_view text = methodToString(HttpMethod::Get);
        benchmark::DoNotOptimize(text);
    }
}
BENCHMARK(bench_method_to_string);
