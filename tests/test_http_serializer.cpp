// HttpSerializer Test Suite
// Validates HTTP response serialization including status line formatting,
// headers, content-length handling, and body placement in raw HTTP output.
//
// Covers:
// - status line formatting (200, 404, etc.)
// - header serialization
// - automatic Content-Length calculation
// - body placement after CRLF separator
// - zero-content-length responses (e.g. 204 No Content)

#include "HTTP/HttpSerializer.h"
#include "HTTP/HttpResponse.h"

#include "test_helper.h"

#include <string>

using namespace FalconHTTP::HTTP;

// Helper function
// Checks if a substring exists within a serialized HTTP response
static bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

// Status line format test
// Ensures HTTP status line is correctly serialized for 200 OK
static void status_line_format() {
    HttpResponse response(HttpStatus::Ok);
    std::string raw = HttpSerializer::serialize(response);

    CHK(raw.substr(0, 15) == "HTTP/1.1 200 OK");
}

// Not Found status line test
// Ensures 404 status is correctly serialized in HTTP response
static void not_found_status_line() {
    HttpResponse response(HttpStatus::NotFound);
    std::string raw = HttpSerializer::serialize(response);

    CHK(raw.substr(0, 22) == "HTTP/1.1 404 Not Found");
}

// Custom header serialization test
// Ensures custom headers are properly included in the HTTP response
static void custom_header_present() {
    HttpResponse response(HttpStatus::Ok);
    response.setHeader("X-Custom", "test-value");
    std::string raw = HttpSerializer::serialize(response);

    CHK(contains(raw, "X-Custom: test-value\r\n") == true);
}

// Automatic Content-Length calculation test
// Ensures response body length is correctly reflected in Content-Length header
static void content_length_auto_calculated() {
    HttpResponse response(HttpStatus::Ok);
    response.setBody("hello world");
    std::string raw = HttpSerializer::serialize(response);

    CHK(contains(raw, "Content-Length: 11\r\n") == true);
}

// Body placement test
// Ensures response body appears after the blank CRLF separator
static void body_appears_after_blank_line() {
    HttpResponse response(HttpStatus::Ok);
    response.setBody("payload-data");
    std::string raw = HttpSerializer::serialize(response);

    std::size_t blankLine = raw.find("\r\n\r\n");
    CHK(blankLine != std::string::npos);

    std::string body = raw.substr(blankLine + 4);
    CHK(body == "payload-data");
}

// Empty body Content-Length test
// Ensures responses with no body correctly report Content-Length: 0
static void empty_body_zero_content_length() {
    HttpResponse response(HttpStatus::NoContent);
    std::string raw = HttpSerializer::serialize(response);

    CHK(contains(raw, "Content-Length: 0\r\n") == true);
}

// Test runner for HttpSerializer
// Executes all HTTP response serialization test cases
void run_http_serializer_tests() {
    setTitle("HttpSerializer Tests");

    RUN(status_line_format);
    RUN(not_found_status_line);
    RUN(custom_header_present);
    RUN(content_length_auto_calculated);
    RUN(body_appears_after_blank_line);
    RUN(empty_body_zero_content_length);

    std::cout << "\n";
}