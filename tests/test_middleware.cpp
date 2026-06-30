// Middleware Test Suite
// Validates middleware behavior including request forwarding, CORS header injection,
// OPTIONS request handling, custom CORS configuration, and exception recovery.
//
// Covers:
// - Logger middleware forwarding
// - CORS header injection
// - OPTIONS request short-circuiting
// - custom CORS origin support
// - successful request recovery pass-through
// - std::exception recovery
// - non-standard exception recovery

#include "Middleware/Logger.h"
#include "Middleware/Cors.h"
#include "Middleware/Recovery.h"
#include "HTTP/HttpRequest.h"
#include "HTTP/HttpResponse.h"
#include "HTTP/HttpStatus.h"

#include "test_helper.h"

#include <stdexcept>
#include <sstream>

using namespace FalconHTTP::Middleware;
using namespace FalconHTTP::HTTP;

// Logger middleware forwarding test
// Ensures the logger middleware forwards execution to the next handler
static void logger_calls_next() {
    std::ostringstream null;

    auto* oldOut = std::cout.rdbuf(null.rdbuf());
    auto* oldErr = std::cerr.rdbuf(null.rdbuf());

    Logger logger;
    bool nextCalled = false;

    NextHandler next([&nextCalled](HttpRequest&, HttpResponse&) {
        nextCalled = true;
        });

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/test");

    HttpResponse response;
    logger(request, response, next);

    std::cout.rdbuf(oldOut);
    std::cerr.rdbuf(oldErr);

    CHK(nextCalled == true);
}

// CORS header injection test
// Ensures CORS headers are added and the request continues to the next handler
static void cors_adds_headers_and_calls_next() {
    Cors cors;
    bool nextCalled = false;

    NextHandler next([&nextCalled](HttpRequest&, HttpResponse&) {
        nextCalled = true;
        });

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/test");

    HttpResponse response;
    cors(request, response, next);

    CHK(nextCalled == true);
    CHK(response.header("Access-Control-Allow-Origin") == "*");
    CHK(response.hasHeader("Access-Control-Allow-Methods") == true);
}

// OPTIONS request handling test
// Ensures OPTIONS requests are handled immediately without invoking the next handler
static void cors_options_short_circuits() {
    Cors cors;
    bool nextCalled = false;

    NextHandler next([&nextCalled](HttpRequest&, HttpResponse&) {
        nextCalled = true;
        });

    HttpRequest request;
    request.setMethod(HttpMethod::Options);
    request.setPath("/test");

    HttpResponse response;
    cors(request, response, next);

    CHK(nextCalled == false);
    CHK(response.status() == HttpStatus::NoContent);
}

// Custom CORS origin test
// Ensures a configured origin is used in the Access-Control-Allow-Origin header
static void cors_custom_origin() {
    Cors cors("https://example.com");

    NextHandler next([](HttpRequest&, HttpResponse&) {});

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/test");

    HttpResponse response;
    cors(request, response, next);

    CHK(response.header("Access-Control-Allow-Origin") == "https://example.com");
}

// Recovery middleware success test
// Ensures requests without exceptions pass through unchanged
static void recovery_passes_through_on_success() {
    Recovery recovery;

    NextHandler next([](HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
        response.setBody("fine");
        });

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/test");

    HttpResponse response;
    recovery(request, response, next);

    CHK(response.status() == HttpStatus::Ok);
    CHK(response.body() == "fine");
}

// Standard exception recovery test
// Ensures std::exception-derived errors are caught and converted into a 500 response
static void recovery_catches_std_exception() {
    std::ostringstream null;

    auto* oldOut = std::cout.rdbuf(null.rdbuf());
    auto* oldErr = std::cerr.rdbuf(null.rdbuf());

    Recovery recovery;

    NextHandler next([](HttpRequest&, HttpResponse&) {
        throw std::runtime_error("simulated failure");
        });

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/test");

    HttpResponse response;
    recovery(request, response, next);

    std::cout.rdbuf(oldOut);
    std::cerr.rdbuf(oldErr);

    CHK(response.status() == HttpStatus::InternalServerError);
    CHK(response.body() == "Internal Server Error");
}

// Non-standard exception recovery test
// Ensures non-standard exceptions are caught and converted into a 500 response
static void recovery_catches_non_standard_exception() {
    std::ostringstream null;

    auto* oldOut = std::cout.rdbuf(null.rdbuf());
    auto* oldErr = std::cerr.rdbuf(null.rdbuf());

    Recovery recovery;

    NextHandler next([](HttpRequest&, HttpResponse&) {
        throw 42;
        });

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/test");

    HttpResponse response;
    recovery(request, response, next);

    std::cout.rdbuf(oldOut);
    std::cerr.rdbuf(oldErr);

    CHK(response.status() == HttpStatus::InternalServerError);
}

// Test runner for Middleware
// Executes all middleware test cases
void run_middleware_tests() {
    setTitle("Middleware Tests");

    RUN(logger_calls_next);
    RUN(cors_adds_headers_and_calls_next);
    RUN(cors_options_short_circuits);
    RUN(cors_custom_origin);
    RUN(recovery_passes_through_on_success);
    RUN(recovery_catches_std_exception);
    RUN(recovery_catches_non_standard_exception);

    std::cout << "\n";
}