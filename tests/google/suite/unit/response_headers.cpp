// HttpResponse header set/get test suite.
//
// Coverage:
// - setHeader()/header() round-trip a value
// - Header names are matched case-insensitively (RFC 7230 §3.2)
// - hasHeader() correctly reports presence/absence
// - headers() exposes entries under their lowercase-normalized key,
//   since that's what HttpSerializer iterates to build the wire output
// - Default-constructed status is HttpStatus::Ok; the explicit-status
//   constructor sets the given status

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::HTTP;

// Verifies a set header can be read back, matched case-insensitively.
TEST(ResponseHeaders, RoundTripsHeaderValue) {
    HttpResponse response;
    response.setHeader("X-Powered-By", "FalconHTTP");

    EXPECT_TRUE(response.hasHeader("x-powered-by"));
    EXPECT_EQ(response.header("X-POWERED-BY"), "FalconHTTP");
}

// Verifies a header that was never set is reported absent.
TEST(ResponseHeaders, ReportsAbsentHeader) {
    HttpResponse response;
    EXPECT_FALSE(response.hasHeader("X-Not-Set"));
    EXPECT_EQ(response.header("X-Not-Set"), "");
}

// Verifies headers() exposes the entry under its lowercase-normalized
// key, since HttpSerializer::appendHeaders() iterates this map
// directly to build the wire response.
TEST(ResponseHeaders, HeadersMapUsesLowercaseKeys) {
    HttpResponse response;
    response.setHeader("Content-Type", "text/plain");

    EXPECT_TRUE(response.headers().contains("content-type"));
    EXPECT_FALSE(response.headers().contains("Content-Type"));
}

// Verifies the default constructor's status is Ok, and the
// explicit-status constructor sets the given status.
TEST(ResponseHeaders, ConstructorsSetExpectedStatus) {
    HttpResponse defaultResponse;
    EXPECT_EQ(defaultResponse.status(), HttpStatus::Ok);

    HttpResponse notFoundResponse(HttpStatus::NotFound);
    EXPECT_EQ(notFoundResponse.status(), HttpStatus::NotFound);
}
