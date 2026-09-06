// methodFromString() / methodToString() test suite.
//
// Coverage:
// - Every known method token parses to its HttpMethod value
// - An unrecognized token parses to HttpMethod::Unknown
// - Matching is exact-case (per RFC 7230, method tokens are
//   case-sensitive) - lowercase tokens do not match
// - methodToString() round-trips back to the original uppercase token
// - methodToString() returns "UNKNOWN" for HttpMethod::Unknown

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::HTTP;

// Verifies every known method token parses to its HttpMethod value.
TEST(HttpMethod, ParsesKnownMethods) {
    EXPECT_EQ(methodFromString("GET"), HttpMethod::Get);
    EXPECT_EQ(methodFromString("POST"), HttpMethod::Post);
    EXPECT_EQ(methodFromString("PUT"), HttpMethod::Put);
    EXPECT_EQ(methodFromString("DELETE"), HttpMethod::Delete);
    EXPECT_EQ(methodFromString("PATCH"), HttpMethod::Patch);
    EXPECT_EQ(methodFromString("HEAD"), HttpMethod::Head);
    EXPECT_EQ(methodFromString("OPTIONS"), HttpMethod::Options);
}

// Verifies an unrecognized token parses to HttpMethod::Unknown.
TEST(HttpMethod, ParsesUnknownMethod) {
    EXPECT_EQ(methodFromString("TRACE"), HttpMethod::Unknown);
    EXPECT_EQ(methodFromString("CONNECT"), HttpMethod::Unknown);
    EXPECT_EQ(methodFromString(""), HttpMethod::Unknown);
}

// Verifies matching is exact-case; lowercase/mixed-case tokens do not
// match, since HTTP method tokens are case-sensitive per RFC 7230.
TEST(HttpMethod, MatchingIsCaseSensitive) {
    EXPECT_EQ(methodFromString("get"), HttpMethod::Unknown);
    EXPECT_EQ(methodFromString("Get"), HttpMethod::Unknown);
    EXPECT_EQ(methodFromString("post"), HttpMethod::Unknown);
}

// Verifies methodToString() returns the expected uppercase token for
// each known method.
TEST(HttpMethod, ConvertsToString) {
    EXPECT_EQ(methodToString(HttpMethod::Get), "GET");
    EXPECT_EQ(methodToString(HttpMethod::Post), "POST");
    EXPECT_EQ(methodToString(HttpMethod::Put), "PUT");
    EXPECT_EQ(methodToString(HttpMethod::Delete), "DELETE");
    EXPECT_EQ(methodToString(HttpMethod::Patch), "PATCH");
    EXPECT_EQ(methodToString(HttpMethod::Head), "HEAD");
    EXPECT_EQ(methodToString(HttpMethod::Options), "OPTIONS");
}

// Verifies methodToString() returns "UNKNOWN" for HttpMethod::Unknown.
TEST(HttpMethod, ConvertsUnknownToString) {
    EXPECT_EQ(methodToString(HttpMethod::Unknown), "UNKNOWN");
}
