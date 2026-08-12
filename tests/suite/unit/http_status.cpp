// statusReasonPhrase() test suite.
//
// Coverage:
// - Every HttpStatus enumerator returns its standard reason phrase
// - The 405, 413, and 431 phrases specifically (added for Router's
//   dispatch() and Server's DoS caps - see the regression suite for
//   the behavior that produces them)

#include <support/framework.h>

using namespace FalconHTTP::HTTP;

// Verifies the common 2xx/4xx codes return their standard phrases.
static void returns_common_reason_phrases() {
    CHK(statusReasonPhrase(HttpStatus::Ok) == "OK");
    CHK(statusReasonPhrase(HttpStatus::Created) == "Created");
    CHK(statusReasonPhrase(HttpStatus::NoContent) == "No Content");
    CHK(statusReasonPhrase(HttpStatus::Found) == "Found");
    CHK(statusReasonPhrase(HttpStatus::BadRequest) == "Bad Request");
    CHK(statusReasonPhrase(HttpStatus::Unauthorized) == "Unauthorized");
    CHK(statusReasonPhrase(HttpStatus::Forbidden) == "Forbidden");
    CHK(statusReasonPhrase(HttpStatus::NotFound) == "Not Found");
}

// Verifies the codes tied to Router::dispatch()'s DispatchResult and
// Server's DoS caps return their correct phrases.
static void returns_dispatch_and_cap_phrases() {
    CHK(statusReasonPhrase(HttpStatus::MethodNotAllowed) == "Method Not Allowed");
    CHK(statusReasonPhrase(HttpStatus::PayloadTooLarge) == "Payload Too Large");
    CHK(statusReasonPhrase(HttpStatus::RequestHeaderFieldsTooLarge) ==
        "Request Header Fields Too Large");
}

// Verifies the 5xx codes return their standard phrases.
static void returns_server_error_phrases() {
    CHK(statusReasonPhrase(HttpStatus::InternalServerError) == "Internal Server Error");
    CHK(statusReasonPhrase(HttpStatus::NotImplemented) == "Not Implemented");
}

// Executes all statusReasonPhrase() test cases.
static void run_tests() {
    RUN(returns_common_reason_phrases);
    RUN(returns_dispatch_and_cap_phrases);
    RUN(returns_server_error_phrases);
}

REGISTER_TEST_SUITE();
