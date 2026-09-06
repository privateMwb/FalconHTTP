// ============================================================
// fuzz/fuzz_path_matcher.cpp
//
// Fuzzer for PathMatcher::match(), targeting the path argument
// specifically - in real usage a route's pattern is developer-
// written (registered once, at startup) while the path is the one
// side of match() an attacker actually controls, arriving fresh with
// every request. This harness fixes a small, representative set of
// patterns (literal segments, single :param segments, and multiple
// params) and fuzzes only the path against each of them, rather than
// fuzzing both sides and mostly generating pattern/path pairs no real
// route table would ever contain.
//
// This codebase's own regression suite already has a path-traversal
// test (tests/suite/integration/... "Rejects Path Traversal
// Attempt") - this harness is the open-ended counterpart to that one
// fixed case.
//
// Not a differential fuzzer (no shadow-model path matcher exists to
// compare against); instead checks the one invariant PathMatcher.h
// itself documents: a true return means every pattern segment matched
// a path segment with nothing left over on either side, so the two
// must have the same segment count.
// ============================================================

#include <FalconHTTP/Routing/PathMatcher.h>

#include <HashMapPro/HashMap.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string_view>

using FalconHTTP::Routing::PathMatcher;
using HashMapPro::HashMap;

namespace {

// A representative handful of route patterns - literal-only, single
// :param, and multiple :param segments - mirroring what an
// application actually registers. Deliberately not fuzzer input:
// patterns are trusted, developer-supplied strings in real usage.
constexpr std::string_view kPatterns[] = {
    "/",
    "/health",
    "/users/:id",
    "/users/:id/posts/:postId",
    "/static/:file",
    "/a/:b/c/:d/e/:f",
};
constexpr std::size_t kPatternCount = sizeof(kPatterns) / sizeof(kPatterns[0]);

std::size_t countSegments(std::string_view s) {
    if (s.empty()) {
        return 0;
    }
    std::size_t count = 1;
    for (char c : s) {
        if (c == '/') {
            ++count;
        }
    }
    return count;
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    if (size == 0) {
        return 0;
    }

    // First byte selects which fixed pattern to match against; the
    // rest of the input is the fuzzed path.
    const std::string_view pattern = kPatterns[data[0] % kPatternCount];
    const std::string_view path(reinterpret_cast<const char*>(data + 1), size - 1);

    HashMap<std::string, std::string> params;
    const bool matched = PathMatcher::match(pattern, path, params);

    if (matched && countSegments(pattern) != countSegments(path)) {
        std::abort();
    }

    return 0;
}
