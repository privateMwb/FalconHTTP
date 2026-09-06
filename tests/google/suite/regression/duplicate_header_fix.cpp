// Regression test: HttpSerializer must not emit a duplicate
// Content-Length or Connection header line when the user has already
// set one via HttpResponse::setHeader().
//
// The original bug: appendHeaders() iterated response.headers() and
// wrote every entry verbatim, THEN unconditionally appended its own
// computed "Content-Length: " and "Connection: close" lines - if a
// handler had called setHeader("Content-Length", ...) or
// setHeader("Connection", ...), the serialized output would contain
// two Content-Length (or two Connection) header lines, which is a
// protocol violation (RFC 7230 forbids duplicate Content-Length).
// Fixed by skipping "content-length"/"connection" entries in the
// user-header loop, since the serializer is authoritative for both.

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::HTTP;

// Verifies a user-set Content-Length does not appear in the output -
// only the serializer's own computed value does.
TEST(DuplicateHeaderFix, UserSetContentLengthIsNotDuplicated) {
    HttpResponse response;
    response.setHeader("Content-Length", "999");
    response.setBody("abc");

    std::string output = HttpSerializer::serialize(response);

    EXPECT_EQ(output.find("999"), std::string::npos);
    EXPECT_NE(output.find("Content-Length: 3\r\n"), std::string::npos);

    std::size_t firstPos = output.find("Content-Length:");
    std::size_t secondPos = output.find("Content-Length:", firstPos + 1);
    EXPECT_EQ(secondPos, std::string::npos);
}

// Verifies a user-set Connection header does not appear in the output
// - only the serializer's own "Connection: close" does.
TEST(DuplicateHeaderFix, UserSetConnectionIsNotDuplicated) {
    HttpResponse response;
    response.setHeader("Connection", "keep-alive");

    std::string output = HttpSerializer::serialize(response);

    EXPECT_EQ(output.find("keep-alive"), std::string::npos);
    EXPECT_NE(output.find("Connection: close\r\n"), std::string::npos);

    std::size_t firstPos = output.find("Connection:");
    std::size_t secondPos = output.find("Connection:", firstPos + 1);
    EXPECT_EQ(secondPos, std::string::npos);
}

// Verifies other user-set headers are unaffected by the fix - only
// Content-Length and Connection are specially handled.
TEST(DuplicateHeaderFix, OtherHeadersStillPassThroughNormally) {
    HttpResponse response;
    response.setHeader("X-Custom", "value");

    std::string output = HttpSerializer::serialize(response);

    EXPECT_NE(output.find("x-custom: value\r\n"), std::string::npos);
}
