// HttpParser Test Suite
// Validates HTTP request parsing including methods, headers, query strings, body parsing,
// URL decoding, and malformed request handling.
//
// Covers:
// - basic GET request parsing
// - multiple header parsing
// - query string extraction
// - URL decoding behavior
// - bare query flags (?debug style)
// - request body extraction
// - malformed request detection

#include "HTTP/HttpParser.h"

#include "test_helper.h"

using namespace FalconHTTP::HTTP;

// Helpers
// Converts LF (\n) line endings into CRLF (\r\n) to simulate real HTTP requests
static std::string crlfRequest(std::string raw) {
    std::string result;
    for (char c : raw) {
        if (c == '\n') {
            result += "\r\n";
        } else {
            result += c;
        }
    }
    return result;
}

// Basic GET request parsing test
// Ensures method, path, version, and headers are correctly parsed
static void simple_get_request() {
    HttpRequest request = HttpParser::parse(crlfRequest(
        "GET /home HTTP/1.1\n"
        "Host: example.com\n"
        "\n"
    ));

    CHK(request.method() == HttpMethod::Get);
    CHK(request.path() == "/home");
    CHK(request.version() == "HTTP/1.1");
    CHK(request.header("Host") == "example.com");
}

// Multiple headers parsing test
// Ensures all headers are parsed and accessible correctly
static void multiple_headers() {
    HttpRequest request = HttpParser::parse(crlfRequest(
        "GET / HTTP/1.1\n"
        "Host: example.com\n"
        "Content-Type: application/json\n"
        "X-Custom: hello\n"
        "\n"
    ));

    CHK(request.header("Host") == "example.com");
    CHK(request.header("Content-Type") == "application/json");
    CHK(request.header("X-Custom") == "hello");
    CHK(request.hasHeader("Nonexistent") == false);
}

// Basic query string parsing test
// Ensures key-value query parameters are extracted correctly
static void query_string_basic() {
    HttpRequest request = HttpParser::parse(crlfRequest(
        "GET /search?q=hello&page=2 HTTP/1.1\n"
        "Host: example.com\n"
        "\n"
    ));

    CHK(request.path() == "/search");
    CHK(request.queryParam("q") == "hello");
    CHK(request.queryParam("page") == "2");
}

// URL decoding test
// Ensures encoded query parameters are correctly decoded
static void query_string_decoding() {
    HttpRequest request = HttpParser::parse(crlfRequest(
        "GET /search?q=hello+world&tag=C%2B%2B HTTP/1.1\n"
        "Host: example.com\n"
        "\n"
    ));

    CHK(request.queryParam("q") == "hello world");
    CHK(request.queryParam("tag") == "C++");
}

// Bare query flag test
// Ensures parameters without values (e.g. ?debug) are handled correctly
static void bare_query_flag() {
    HttpRequest request = HttpParser::parse(crlfRequest(
        "GET /search?debug HTTP/1.1\n"
        "Host: example.com\n"
        "\n"
    ));

    CHK(request.hasQueryParam("debug") == true);
    CHK(request.queryParam("debug") == "");
}

// Body extraction test
// Ensures POST request body is correctly parsed using Content-Length
static void body_extraction() {
    HttpRequest request = HttpParser::parse(crlfRequest(
        "POST /submit HTTP/1.1\n"
        "Host: example.com\n"
        "Content-Length: 13\n"
        "\n"
        "hello, world!"
    ));

    CHK(request.method() == HttpMethod::Post);
    CHK(request.body() == "hello, world!");
}

// Malformed request test
// Ensures parser throws on invalid HTTP formatting (missing blank line separator)
static void malformed_missing_blank_line() {
    bool threw = false;
    try{
        (void)HttpParser::parse(crlfRequest(
            "HET / HTTP/1.1.\n"
            "Host: example.com\n"
        ));
    } catch (const std::exception&) {
        threw = true;
    }
    CHK(threw == true);
}

// Test runner for HttpParser
// Executes all HTTP parsing test cases
void run_http_parser_tests() {
    setTitle("HttpParser Tests");

    RUN(simple_get_request);
    RUN(multiple_headers);
    RUN(query_string_basic);
    RUN(query_string_decoding);
    RUN(bare_query_flag);
    RUN(body_extraction);
    RUN(malformed_missing_blank_line);

    std::cout<< "\n";
}