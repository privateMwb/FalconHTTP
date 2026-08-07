// methodFromString() / methodToString() test suite.
//
// Coverage:
// - Every known method token parses to its HttpMethod value
// - An unrecognized token parses to HttpMethod::Unknown
// - Matching is exact-case (per RFC 7230, method tokens are
//   case-sensitive) - lowercase tokens do not match
// - methodToString() round-trips back to the original uppercase token
// - methodToString() returns "UNKNOWN" for HttpMethod::Unknown

#include <support/framework.h>

using namespace FalconHTTP::HTTP;

// Verifies every known method token parses to its HttpMethod value.
static void parses_known_methods() {
    CHK(methodFromString("GET") == HttpMethod::Get);
    CHK(methodFromString("POST") == HttpMethod::Post);
    CHK(methodFromString("PUT") == HttpMethod::Put);
    CHK(methodFromString("DELETE") == HttpMethod::Delete);
    CHK(methodFromString("PATCH") == HttpMethod::Patch);
    CHK(methodFromString("HEAD") == HttpMethod::Head);
    CHK(methodFromString("OPTIONS") == HttpMethod::Options);
}

// Verifies an unrecognized token parses to HttpMethod::Unknown.
static void parses_unknown_method() {
    CHK(methodFromString("TRACE") == HttpMethod::Unknown);
    CHK(methodFromString("CONNECT") == HttpMethod::Unknown);
    CHK(methodFromString("") == HttpMethod::Unknown);
}

// Verifies matching is exact-case; lowercase/mixed-case tokens do not
// match, since HTTP method tokens are case-sensitive per RFC 7230.
static void matching_is_case_sensitive() {
    CHK(methodFromString("get") == HttpMethod::Unknown);
    CHK(methodFromString("Get") == HttpMethod::Unknown);
    CHK(methodFromString("post") == HttpMethod::Unknown);
}

// Verifies methodToString() returns the expected uppercase token for
// each known method.
static void converts_to_string() {
    CHK(methodToString(HttpMethod::Get) == "GET");
    CHK(methodToString(HttpMethod::Post) == "POST");
    CHK(methodToString(HttpMethod::Put) == "PUT");
    CHK(methodToString(HttpMethod::Delete) == "DELETE");
    CHK(methodToString(HttpMethod::Patch) == "PATCH");
    CHK(methodToString(HttpMethod::Head) == "HEAD");
    CHK(methodToString(HttpMethod::Options) == "OPTIONS");
}

// Verifies methodToString() returns "UNKNOWN" for HttpMethod::Unknown.
static void converts_unknown_to_string() {
    CHK(methodToString(HttpMethod::Unknown) == "UNKNOWN");
}

// Executes all method conversion test cases.
static void run_tests() {
    RUN(parses_known_methods);
    RUN(parses_unknown_method);
    RUN(matching_is_case_sensitive);
    RUN(converts_to_string);
    RUN(converts_unknown_to_string);
}

REGISTER_TEST_SUITE();
