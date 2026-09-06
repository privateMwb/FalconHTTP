// ============================================================
// fuzz/fuzz_http_parser.cpp
//
// Crash-oracle fuzzer for HttpParser::parse(), the entry point for
// every byte FalconHTTP ever reads off a socket before any of the
// library's own validation has had a chance to run. Unlike
// fuzz_vector.cpp in VectorPro, this is NOT a differential fuzzer -
// there's no shadow-model HTTP parser to compare against - so this
// instead (a) treats parse()'s own documented std::runtime_error as
// an expected, non-finding outcome for malformed input, and (b) on a
// successful parse, exercises every HttpRequest accessor so
// ASan/UBSan can catch anything reachable only through a specific
// method/path/header/query/body combination that a hand-written unit
// test wouldn't think to construct.
//
// This codebase's own regression suite
// (bad_content_length/duplicate_header_fix/body_offset_fix/
// oversized_body_rejected/oversized_header_rejected, all under
// tests/suite/regression/) is a track record of exactly the kind of
// malformed-input bug this harness is meant to catch before a human
// has to find the next one by hand.
//
// Specifically exercised:
//   - the request line, header block, and body all in one buffer,
//     with no assumption about where line boundaries or the
//     Content-Length actually fall - the fuzzer controls all of it
//   - HttpRequest::json() on the parsed body - JsonPro's parser is a
//     second parsing surface reachable from exactly this code path in
//     production (any route handler calling request.json())
//   - header/query/path-param lookups by name, both present and
//     absent, exercising HashMap's case-normalization and hashing on
//     arbitrary fuzzer-supplied bytes
//
// Deliberately NOT covered here: HttpSerializer (operates on a
// response FalconHTTP's own code already built, not attacker bytes),
// Router::dispatch()/matchStream() and SseConnection (operate on an
// already-parsed HttpRequest, not raw bytes), and PathMatcher (see
// fuzz_path_matcher.cpp instead - a separate harness, since the
// attacker-controlled input there is just the path segment, not a
// full request).
// ============================================================

#include <FalconHTTP/HTTP/HttpParser.h>
#include <FalconHTTP/HTTP/HttpRequest.h>

#include <cstdint>
#include <cstddef>
#include <exception>
#include <string_view>

using FalconHTTP::HTTP::HttpParser;
using FalconHTTP::HTTP::HttpRequest;

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    const std::string_view raw(reinterpret_cast<const char*>(data), size);

    HttpRequest request;
    try {
        request = HttpParser::parse(raw);
    } catch (const std::exception&) {
        // Documented behavior (HttpParser.h): a missing CRLF, a
        // request line without both a method and version, or a
        // header line without a colon all throw std::runtime_error.
        // That's the parser correctly rejecting malformed input, not
        // a finding.
        return 0;
    }

    // A successful parse must leave every accessor safely callable,
    // regardless of what bytes produced it.
    (void)request.method();
    (void)request.path();
    (void)request.version();
    (void)request.body();

    (void)request.hasHeader("Content-Type");
    (void)request.header("Content-Length");
    (void)request.hasHeader("");
    (void)request.header("");

    (void)request.hasQueryParam("q");
    (void)request.queryParam("q");

    (void)request.hasPathParam("id");
    (void)request.pathParam("id");

    // json() is documented to propagate whatever JsonPro::Json::parse()
    // throws on malformed bodies (HttpRequest.h) - an external-library
    // contract this harness doesn't own, so any std::exception here is
    // expected. Only a crash or an escaping non-std::exception would
    // be a finding.
    try {
        (void)request.json();
    } catch (const std::exception&) {
    }

    return 0;
}
