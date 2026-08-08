// FalconHTTP HTTP Method Conversion Benchmark Suite
// Measures methodFromString() and methodToString() performance.
//
// methodFromString() runs once per parsed request (see
// HttpParser::parseRequestLine()) - a simple, frequently-called
// string comparison chain worth a floor number for.
//
// NOTE: no comparison baseline exists for FalconHTTP (see the suite
// README) - this uses BENCH() with a single lambda, timing FalconHTTP
// alone.
//
// Covers:
// - methodFromString() on GET (checked first, best case)
// - methodFromString() on OPTIONS (checked last, worst case)
// - methodToString() on HttpMethod::Get

#include <support/framework.h>

using namespace FalconHTTP::HTTP;

// Measures methodFromString() on GET - the first method checked,
// so this is the best-case comparison chain cost.
static void bench_method_from_string_best_case() {
    auto f = [&] {
        HttpMethod method = methodFromString("GET");
        (void)method;
    };

    BENCH("methodFromString() GET (best case)", f);
}

// Measures methodFromString() on OPTIONS - the last method checked,
// so this is the worst-case comparison chain cost.
static void bench_method_from_string_worst_case() {
    auto f = [&] {
        HttpMethod method = methodFromString("OPTIONS");
        (void)method;
    };

    BENCH("methodFromString() OPTIONS (worst case)", f);
}

// Measures methodToString() on HttpMethod::Get.
static void bench_method_to_string() {
    auto f = [&] {
        std::string_view text = methodToString(HttpMethod::Get);
        (void)text;
    };

    BENCH("methodToString() Get", f);
}

// Executes all HTTP method conversion benchmark cases.
static void run_benchmarks() {
    bench_method_from_string_best_case();
    std::cout << "\n";

    bench_method_from_string_worst_case();
    std::cout << "\n";

    bench_method_to_string();
}

REGISTER_BENCH_SUITE();
