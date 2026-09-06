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

#include <support/framework.h>

using namespace FalconHTTP::HTTP;

// Verifies a set header can be read back with the exact same name.
static void round_trips_header_value() {
    HttpRequest request;
    request.setHeader("X-Request-Id", "abc123");
    CHK(request.hasHeader("X-Request-Id"));
    CHK(request.header("X-Request-Id") == "abc123");
}

// Verifies header lookup is case-insensitive in both directions:
// setting mixed-case and reading lowercase, and vice versa.
static void header_lookup_is_case_insensitive() {
    HttpRequest request;
    request.setHeader("Content-Type", "application/json");

    CHK(request.hasHeader("content-type"));
    CHK(request.hasHeader("CONTENT-TYPE"));
    CHK(request.header("content-type") == "application/json");
    CHK(request.header("CoNtEnT-tYpE") == "application/json");
}

// Verifies a header that was never set is reported absent, and
// header() returns an empty string for it.
static void reports_absent_header() {
    HttpRequest request;
    CHK(!request.hasHeader("X-Not-Set"));
    CHK(request.header("X-Not-Set") == "");
}

// Verifies setting the same header name with different casing updates
// the single stored entry rather than creating a duplicate - the most
// recent value wins and only one logical entry exists.
static void same_header_different_case_updates_single_entry() {
    HttpRequest request;
    request.setHeader("Accept", "text/html");
    request.setHeader("ACCEPT", "application/json");

    CHK(request.header("accept") == "application/json");
}

// Executes all HttpRequest header test cases.
static void run_tests() {
    RUN(round_trips_header_value);
    RUN(header_lookup_is_case_insensitive);
    RUN(reports_absent_header);
    RUN(same_header_different_case_updates_single_entry);
}

REGISTER_TEST_SUITE();
