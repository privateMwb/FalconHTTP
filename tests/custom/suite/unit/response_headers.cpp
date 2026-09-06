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

#include <support/framework.h>

using namespace FalconHTTP::HTTP;

// Verifies a set header can be read back, matched case-insensitively.
static void round_trips_header_value() {
    HttpResponse response;
    response.setHeader("X-Powered-By", "FalconHTTP");

    CHK(response.hasHeader("x-powered-by"));
    CHK(response.header("X-POWERED-BY") == "FalconHTTP");
}

// Verifies a header that was never set is reported absent.
static void reports_absent_header() {
    HttpResponse response;
    CHK(!response.hasHeader("X-Not-Set"));
    CHK(response.header("X-Not-Set") == "");
}

// Verifies headers() exposes the entry under its lowercase-normalized
// key, since HttpSerializer::appendHeaders() iterates this map
// directly to build the wire response.
static void headers_map_uses_lowercase_keys() {
    HttpResponse response;
    response.setHeader("Content-Type", "text/plain");

    CHK(response.headers().contains("content-type"));
    CHK(!response.headers().contains("Content-Type"));
}

// Verifies the default constructor's status is Ok, and the
// explicit-status constructor sets the given status.
static void constructors_set_expected_status() {
    HttpResponse defaultResponse;
    CHK(defaultResponse.status() == HttpStatus::Ok);

    HttpResponse notFoundResponse(HttpStatus::NotFound);
    CHK(notFoundResponse.status() == HttpStatus::NotFound);
}

// Executes all HttpResponse header/constructor test cases.
static void run_tests() {
    RUN(round_trips_header_value);
    RUN(reports_absent_header);
    RUN(headers_map_uses_lowercase_keys);
    RUN(constructors_set_expected_status);
}

REGISTER_TEST_SUITE();
