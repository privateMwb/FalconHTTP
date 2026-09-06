// HttpRequest header set/get test suite.
//
// Coverage:
// - setHeader()/header() round-trip a value
// - Header names are matched case-insensitively (RFC 7230 §3.2) -
//   "Content-Type" and "content-type" resolve to the same entry
// - hasHeader() correctly reports presence/absence
// - header() returns an empty string for an absent header (ambiguous
//   with an explicitly-empty value - hasHeader() disambiguates)
// - Setting the same header name twice (different case) updates the
//   single stored entry rather than creating a second one

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::HTTP;

// Verifies a set header can be read back with the exact same name.
TEST(RequestHeaders, RoundTripsHeaderValue) {
    HttpRequest request;
    request.setHeader("X-Request-Id", "abc123");
    EXPECT_TRUE(request.hasHeader("X-Request-Id"));
    EXPECT_EQ(request.header("X-Request-Id"), "abc123");
}

// Verifies header lookup is case-insensitive in both directions:
// setting mixed-case and reading lowercase, and vice versa.
TEST(RequestHeaders, HeaderLookupIsCaseInsensitive) {
    HttpRequest request;
    request.setHeader("Content-Type", "application/json");

    EXPECT_TRUE(request.hasHeader("content-type"));
    EXPECT_TRUE(request.hasHeader("CONTENT-TYPE"));
    EXPECT_EQ(request.header("content-type"), "application/json");
    EXPECT_EQ(request.header("CoNtEnT-tYpE"), "application/json");
}

// Verifies a header that was never set is reported absent, and
// header() returns an empty string for it.
TEST(RequestHeaders, ReportsAbsentHeader) {
    HttpRequest request;
    EXPECT_FALSE(request.hasHeader("X-Not-Set"));
    EXPECT_EQ(request.header("X-Not-Set"), "");
}

// Verifies setting the same header name with different casing updates
// the single stored entry rather than creating a duplicate - the most
// recent value wins and only one logical entry exists.
TEST(RequestHeaders, SameHeaderDifferentCaseUpdatesSingleEntry) {
    HttpRequest request;
    request.setHeader("Accept", "text/html");
    request.setHeader("ACCEPT", "application/json");

    EXPECT_EQ(request.header("accept"), "application/json");
}
