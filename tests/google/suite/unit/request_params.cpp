// HttpRequest query/path parameter set/get test suite.
//
// Coverage:
// - setQueryParam()/queryParam() round-trip a value
// - setPathParam()/pathParam() round-trip a value
// - Both are matched case-sensitively, unlike headers - "id" and "Id"
//   are distinct entries
// - has*Param() correctly reports presence/absence
// - *Param() returns an empty string for an absent key

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::HTTP;

// Verifies a set query parameter can be read back.
TEST(RequestParams, RoundTripsQueryParam) {
    HttpRequest request;
    request.setQueryParam("page", "2");

    EXPECT_TRUE(request.hasQueryParam("page"));
    EXPECT_EQ(request.queryParam("page"), "2");
}

// Verifies a set path parameter can be read back.
TEST(RequestParams, RoundTripsPathParam) {
    HttpRequest request;
    request.setPathParam("id", "42");

    EXPECT_TRUE(request.hasPathParam("id"));
    EXPECT_EQ(request.pathParam("id"), "42");
}

// Verifies query and path parameter names are matched case-sensitively,
// unlike header names - "id" and "ID" are distinct entries.
TEST(RequestParams, ParamsAreCaseSensitive) {
    HttpRequest request;
    request.setQueryParam("sort", "asc");

    EXPECT_TRUE(request.hasQueryParam("sort"));
    EXPECT_FALSE(request.hasQueryParam("Sort"));
    EXPECT_FALSE(request.hasQueryParam("SORT"));
}

// Verifies a query/path parameter that was never set is reported
// absent, and the accessor returns an empty string for it.
TEST(RequestParams, ReportsAbsentParam) {
    HttpRequest request;

    EXPECT_FALSE(request.hasQueryParam("missing"));
    EXPECT_EQ(request.queryParam("missing"), "");

    EXPECT_FALSE(request.hasPathParam("missing"));
    EXPECT_EQ(request.pathParam("missing"), "");
}
