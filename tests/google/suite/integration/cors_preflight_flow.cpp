// CORS preflight flow integration test suite.
//
// Coverage:
// - An OPTIONS request short-circuits Cors's operator() with a 204
//   response and does NOT call `next` - verified with the same
//   NextHandler contract Server::runChain() uses to continue the chain
// - A non-OPTIONS request gets the CORS headers AND continues to `next`
// - The configured allowed-origin value is reflected in the response

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Middleware;

// Verifies an OPTIONS request receives 204 and the chain does not
// continue past Cors - `next` must not be invoked.
TEST(CorsPreflightFlow, OptionsRequestShortCircuitsWith204) {
    Cors cors("https://example.com");
    bool nextCalled = false;

    HttpRequest request;
    request.setMethod(HttpMethod::Options);

    HttpResponse response;
    NextHandler next = [&nextCalled](HttpRequest&, HttpResponse&) { nextCalled = true; };

    cors(request, response, next);

    EXPECT_FALSE(nextCalled);
    EXPECT_EQ(response.status(), HttpStatus::NoContent);
}

// Verifies a non-OPTIONS request still receives the CORS headers, but
// the chain continues - `next` is invoked.
TEST(CorsPreflightFlow, NonOptionsRequestGetsHeadersAndContinues) {
    Cors cors("https://example.com");
    bool nextCalled = false;

    HttpRequest request;
    request.setMethod(HttpMethod::Get);

    HttpResponse response;
    NextHandler next = [&nextCalled](HttpRequest&, HttpResponse&) { nextCalled = true; };

    cors(request, response, next);

    EXPECT_TRUE(nextCalled);
    EXPECT_EQ(response.header("access-control-allow-origin"), "https://example.com");
}

// Verifies the configured allowed-origin value (rather than the
// default "*") is what gets reflected in the response.
TEST(CorsPreflightFlow, ReflectsConfiguredAllowedOrigin) {
    Cors defaultCors;
    Cors customCors("https://my-app.example");

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    NextHandler next = [](HttpRequest&, HttpResponse&) {};

    HttpResponse defaultResponse;
    defaultCors(request, defaultResponse, next);
    EXPECT_EQ(defaultResponse.header("access-control-allow-origin"), "*");

    HttpResponse customResponse;
    customCors(request, customResponse, next);
    EXPECT_EQ(customResponse.header("access-control-allow-origin"), "https://my-app.example");
}
