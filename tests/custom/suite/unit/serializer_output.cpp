// HttpSerializer::serialize() test suite.
//
// Coverage:
// - Status line format ("HTTP/1.1 {code} {reason}\r\n")
// - User-set headers are included in the output
// - Content-Length is always computed from the actual body size
// - Connection: close is always present (no keep-alive support)
// - A user-set Content-Length or Connection header does not produce a
//   duplicate line - HttpSerializer is authoritative for both
// - The body follows the blank line terminating the headers

#include <support/framework.h>

using namespace FalconHTTP::HTTP;

// Verifies the status line matches "HTTP/1.1 {code} {reason}\r\n".
static void writes_correct_status_line() {
    HttpResponse response(HttpStatus::NotFound);
    std::string output = HttpSerializer::serialize(response);

    CHK(output.starts_with("HTTP/1.1 404 Not Found\r\n"));
}

// Verifies a user-set header appears in the serialized output.
static void includes_user_set_headers() {
    HttpResponse response;
    response.setHeader("X-Custom", "value");

    std::string output = HttpSerializer::serialize(response);

    CHK(output.find("x-custom: value\r\n") != std::string::npos);
}

// Verifies Content-Length always reflects the actual body size,
// computed by the serializer rather than trusted from user input.
static void computes_content_length_from_body() {
    HttpResponse response;
    response.setBody("hello world");

    std::string output = HttpSerializer::serialize(response);

    CHK(output.find("Content-Length: 11\r\n") != std::string::npos);
    CHK(output.ends_with("hello world"));
}

// Verifies Connection: close is always present, since FalconHTTP does
// not implement keep-alive.
static void always_includes_connection_close() {
    HttpResponse response;
    std::string output = HttpSerializer::serialize(response);

    CHK(output.find("Connection: close\r\n") != std::string::npos);
}

// Verifies a user-set Content-Length or Connection header is not
// emitted twice - HttpSerializer's own computed values are the only
// ones present in the output.
static void does_not_duplicate_reserved_headers() {
    HttpResponse response;
    response.setHeader("Content-Length", "999");
    response.setHeader("Connection", "keep-alive");
    response.setBody("abc");

    std::string output = HttpSerializer::serialize(response);

    std::size_t contentLengthCount = 0;
    std::size_t connectionCount = 0;
    std::size_t pos = 0;

    while ((pos = output.find("content-length:", pos)) != std::string::npos) {
        ++contentLengthCount;
        pos += 1;
    }
    pos = 0;
    while ((pos = output.find("Connection:", pos)) != std::string::npos) {
        ++connectionCount;
        pos += 1;
    }

    CHK(contentLengthCount <= 1);
    CHK(connectionCount == 1);
    CHK(output.find("Content-Length: 3\r\n") != std::string::npos);
    CHK(output.find("Connection: close\r\n") != std::string::npos);
    CHK(output.find("999") == std::string::npos);
    CHK(output.find("keep-alive") == std::string::npos);
}

// Executes all HttpSerializer::serialize() test cases.
static void run_tests() {
    RUN(writes_correct_status_line);
    RUN(includes_user_set_headers);
    RUN(computes_content_length_from_body);
    RUN(always_includes_connection_close);
    RUN(does_not_duplicate_reserved_headers);
}

REGISTER_TEST_SUITE();
