// HttpResponse::setJson() test suite.
//
// Coverage:
// - setJson() sets the body to the JSON value's dump() output
// - setJson() sets Content-Type to application/json
// - setJson() overwrites any body/Content-Type set beforehand
//
// NOTE: this suite only assumes JsonPro::Json is default-constructible
// and has a dump() method (both directly confirmed against
// HttpResponse.cpp's setJson() implementation: `body_ = json.dump();`).
// It deliberately does NOT assume any particular API for populating a
// Json with keys/values (e.g. operator[], emplace()), since that
// wasn't visible in the reviewed source - a default-constructed,
// empty Json is used throughout instead. Extend this suite with
// populated-object cases once JsonPro's construction API is confirmed.

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::HTTP;

// Verifies setJson() sets the body to the JSON value's own dump()
// output, so the two stay consistent by construction.
TEST(ResponseJson, SetsBodyToJsonDump) {
    HttpResponse response;
    Json json;

    response.setJson(json);

    EXPECT_EQ(response.body(), json.dump());
}

// Verifies setJson() sets Content-Type to application/json.
TEST(ResponseJson, SetsJsonContentType) {
    HttpResponse response;
    Json json;

    response.setJson(json);

    EXPECT_TRUE(response.hasHeader("content-type"));
    EXPECT_EQ(response.header("content-type"), "application/json");
}

// Verifies setJson() overwrites a body/Content-Type set beforehand.
TEST(ResponseJson, OverwritesPriorBodyAndContentType) {
    HttpResponse response;
    response.setBody("not json");
    response.setHeader("Content-Type", "text/plain");

    Json json;
    response.setJson(json);

    EXPECT_EQ(response.body(), json.dump());
    EXPECT_EQ(response.header("content-type"), "application/json");
}
