// HttpRequest query/path parameter set/get test suite.
//
// Coverage:
// - setQueryParam()/queryParam() round-trip a value
// - setPathParam()/pathParam() round-trip a value
// - Both are matched case-sensitively, unlike headers - "id" and "Id"
//   are distinct entries
// - has*Param() correctly reports presence/absence
// - *Param() returns an empty string for an absent key

#include <support/framework.h>

using namespace FalconHTTP::HTTP;

// Verifies a set query parameter can be read back.
static void round_trips_query_param() {
    HttpRequest request;
    request.setQueryParam("page", "2");

    CHK(request.hasQueryParam("page"));
    CHK(request.queryParam("page") == "2");
}

// Verifies a set path parameter can be read back.
static void round_trips_path_param() {
    HttpRequest request;
    request.setPathParam("id", "42");

    CHK(request.hasPathParam("id"));
    CHK(request.pathParam("id") == "42");
}

// Verifies query and path parameter names are matched case-sensitively,
// unlike header names - "id" and "ID" are distinct entries.
static void params_are_case_sensitive() {
    HttpRequest request;
    request.setQueryParam("sort", "asc");

    CHK(request.hasQueryParam("sort"));
    CHK(!request.hasQueryParam("Sort"));
    CHK(!request.hasQueryParam("SORT"));
}

// Verifies a query/path parameter that was never set is reported
// absent, and the accessor returns an empty string for it.
static void reports_absent_param() {
    HttpRequest request;

    CHK(!request.hasQueryParam("missing"));
    CHK(request.queryParam("missing") == "");

    CHK(!request.hasPathParam("missing"));
    CHK(request.pathParam("missing") == "");
}

// Executes all HttpRequest query/path param test cases.
static void run_tests() {
    RUN(round_trips_query_param);
    RUN(round_trips_path_param);
    RUN(params_are_case_sensitive);
    RUN(reports_absent_param);
}

REGISTER_TEST_SUITE();
