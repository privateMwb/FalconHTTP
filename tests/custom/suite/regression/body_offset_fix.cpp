// Regression test: body offset must be headerEnd + 4, not headerEnd
// overwritten to 4.
//
// The original bug (Server.cpp): `std::size_t bodyStart = headerEnd =
// 4;` is a chained assignment - it overwrote headerEnd itself with
// the literal 4 instead of computing headerEnd + 4, corrupting the
// body-bytes-received tracking used by handleConnection()'s read
// loop and causing bodies to be truncated or dropped for virtually
// any real request.
//
// SCOPE LIMITATION: the buggy code lived inside
// Server::handleConnection(), which is private and only reachable via
// a live accepted connection - there is no way to drive it directly
// through the public API. This test instead verifies the property
// the fix guarantees at the HttpParser level: the body is extracted
// correctly regardless of how long the header block is, which is the
// same class of "offset past the header block" computation the bug
// got wrong. It does not execute the originally-buggy line itself.
// See connection_close_sent.cpp / oversized_body_rejected.cpp for
// this suite's approach to testing Server::handleConnection() via a
// live socket where that's unavoidable.

#include <support/framework.h>

using namespace FalconHTTP::HTTP;

// Verifies the body is extracted correctly for a request with a
// short header block.
static void extracts_body_with_short_header_block() {
    std::string raw = "POST /x HTTP/1.1\r\nContent-Length: 3\r\n\r\nabc";
    HttpRequest request = HttpParser::parse(raw);
    CHK(request.body() == "abc");
}

// Verifies the body is extracted correctly for a request with a much
// longer header block - if any offset were miscalculated as a fixed
// constant rather than relative to the actual header length (the
// original bug's exact failure mode), this is where it would show.
static void extracts_body_with_long_header_block() {
    std::string raw = "POST /x HTTP/1.1\r\n"
                      "Host: example.com\r\n"
                      "User-Agent: FalconHTTP-Regression-Test/1.0\r\n"
                      "Accept: */*\r\n"
                      "X-Custom-Header-One: some-value-here\r\n"
                      "X-Custom-Header-Two: another-value-here\r\n"
                      "Content-Type: text/plain\r\n"
                      "Content-Length: 3\r\n"
                      "\r\n"
                      "xyz";

    HttpRequest request = HttpParser::parse(raw);
    CHK(request.body() == "xyz");
}

// Verifies an empty body (Content-Length: 0, immediate blank line)
// is extracted as empty rather than picking up trailing garbage.
static void extracts_empty_body_correctly() {
    std::string raw = "GET /x HTTP/1.1\r\nHost: h\r\n\r\n";
    HttpRequest request = HttpParser::parse(raw);
    CHK(request.body() == "");
}

// Executes all body offset regression test cases.
static void run_tests() {
    RUN(extracts_body_with_short_header_block);
    RUN(extracts_body_with_long_header_block);
    RUN(extracts_empty_body_correctly);
}

REGISTER_TEST_SUITE();
