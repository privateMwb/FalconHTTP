// Regression test: header lookup must be case-insensitive.
//
// The original bug: header names were stored and compared exactly as
// received, with no case normalization - "Content-Type" (as most real
// clients send it) and a lookup for "content-type" were treated as
// two different entries, so hasHeader()/header() calls using the
// "wrong" case would silently miss a header that was actually present.
// Fixed by normalizing header names to lowercase on both insert and
// lookup, in both HttpRequest and HttpResponse.
//
// See unit/request_headers.cpp and unit/response_headers.cpp for
// broader header-handling coverage; this suite pins the specific
// case-insensitivity regression on both classes.

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::HTTP;

// Verifies a header set with typical mixed-case ("Content-Type") is
// found via a differently-cased lookup on HttpRequest.
TEST(HeaderCaseLookup, RequestHeaderLookupSurvivesCaseMismatch) {
    HttpRequest request;
    request.setHeader("Content-Type", "application/json");

    EXPECT_TRUE(request.hasHeader("content-type"));
    EXPECT_EQ(request.header("content-type"), "application/json");
}

// Verifies the same on HttpResponse.
TEST(HeaderCaseLookup, ResponseHeaderLookupSurvivesCaseMismatch) {
    HttpResponse response;
    response.setHeader("Content-Type", "application/json");

    EXPECT_TRUE(response.hasHeader("content-type"));
    EXPECT_EQ(response.header("content-type"), "application/json");
}
