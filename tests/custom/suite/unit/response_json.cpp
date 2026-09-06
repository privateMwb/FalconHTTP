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

#include <support/framework.h>

using namespace FalconHTTP::HTTP;

// Verifies setJson() sets the body to the JSON value's own dump()
// output, so the two stay consistent by construction.
static void sets_body_to_json_dump() {
    HttpResponse response;
    Json json;

    response.setJson(json);

    CHK(response.body() == json.dump());
}

// Verifies setJson() sets Content-Type to application/json.
static void sets_json_content_type() {
    HttpResponse response;
    Json json;

    response.setJson(json);

    CHK(response.hasHeader("content-type"));
    CHK(response.header("content-type") == "application/json");
}

// Verifies setJson() overwrites a body/Content-Type set beforehand.
static void overwrites_prior_body_and_content_type() {
    HttpResponse response;
    response.setBody("not json");
    response.setHeader("Content-Type", "text/plain");

    Json json;
    response.setJson(json);

    CHK(response.body() == json.dump());
    CHK(response.header("content-type") == "application/json");
}

// Executes all HttpResponse::setJson() test cases.
static void run_tests() {
    RUN(sets_body_to_json_dump);
    RUN(sets_json_content_type);
    RUN(overwrites_prior_body_and_content_type);
}

REGISTER_TEST_SUITE();
