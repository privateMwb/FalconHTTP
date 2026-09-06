// statusReasonPhrase() test suite.
//
// Coverage:
// - Every HttpStatus enumerator returns its standard reason phrase
// - The 405, 413, and 431 phrases specifically (added for Router's
//   dispatch() and Server's DoS caps - see the regression suite for
//   the behavior that produces them)

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::HTTP;

// Verifies the common 2xx/4xx codes return their standard phrases.
TEST(HttpStatus, ReturnsCommonReasonPhrases) {
    EXPECT_EQ(statusReasonPhrase(HttpStatus::Ok), "OK");
    EXPECT_EQ(statusReasonPhrase(HttpStatus::Created), "Created");
    EXPECT_EQ(statusReasonPhrase(HttpStatus::NoContent), "No Content");
    EXPECT_EQ(statusReasonPhrase(HttpStatus::Found), "Found");
    EXPECT_EQ(statusReasonPhrase(HttpStatus::BadRequest), "Bad Request");
    EXPECT_EQ(statusReasonPhrase(HttpStatus::Unauthorized), "Unauthorized");
    EXPECT_EQ(statusReasonPhrase(HttpStatus::Forbidden), "Forbidden");
    EXPECT_EQ(statusReasonPhrase(HttpStatus::NotFound), "Not Found");
}

// Verifies the codes tied to Router::dispatch()'s DispatchResult and
// Server's DoS caps return their correct phrases.
TEST(HttpStatus, ReturnsDispatchAndCapPhrases) {
    EXPECT_EQ(statusReasonPhrase(HttpStatus::MethodNotAllowed), "Method Not Allowed");
    EXPECT_EQ(statusReasonPhrase(HttpStatus::PayloadTooLarge), "Payload Too Large");
    EXPECT_EQ(statusReasonPhrase(HttpStatus::RequestHeaderFieldsTooLarge),
              "Request Header Fields Too Large");
}

// Verifies the code tied to RateLimiterMiddleware's rate-limit
// short-circuit returns its correct phrase.
TEST(HttpStatus, ReturnsRateLimitPhrase) {
    EXPECT_EQ(statusReasonPhrase(HttpStatus::TooManyRequests), "Too Many Requests");
}

// Verifies the 5xx codes return their standard phrases.
TEST(HttpStatus, ReturnsServerErrorPhrases) {
    EXPECT_EQ(statusReasonPhrase(HttpStatus::InternalServerError), "Internal Server Error");
    EXPECT_EQ(statusReasonPhrase(HttpStatus::NotImplemented), "Not Implemented");
}
