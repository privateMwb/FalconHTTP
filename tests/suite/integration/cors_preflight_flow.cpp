// CORS preflight flow integration test suite.
//
// Coverage:
// - An OPTIONS request short-circuits Cors's operator() with a 204
//   response and does NOT call `next` - verified with the same
//   NextHandler contract Server::runChain() uses to continue the chain
// - A non-OPTIONS request gets the CORS headers AND continues to `next`
// - The configured allowed-origin value is reflected in the response

#include <support/framework.h>

using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Middleware;

// Verifies an OPTIONS request receives 204 and the chain does not
// continue past Cors - `next` must not be invoked.
static void options_request_short_circuits_with_204() {
    Cors cors("https://example.com");
    bool nextCalled = false;

    HttpRequest request;
    request.setMethod(HttpMethod::Options);

    HttpResponse response;
    NextHandler next = [&nextCalled](HttpRequest&, HttpResponse&) { nextCalled = true; };

    cors(request, response, next);

    CHK(!nextCalled);
    CHK(response.status() == HttpStatus::NoContent);
}

// Verifies a non-OPTIONS request still receives the CORS headers, but
// the chain continues - `next` is invoked.
static void non_options_request_gets_headers_and_continues() {
    Cors cors("https://example.com");
    bool nextCalled = false;

    HttpRequest request;
    request.setMethod(HttpMethod::Get);

    HttpResponse response;
    NextHandler next = [&nextCalled](HttpRequest&, HttpResponse&) { nextCalled = true; };

    cors(request, response, next);

    CHK(nextCalled);
    CHK(response.header("access-control-allow-origin") == "https://example.com");
}

// Verifies the configured allowed-origin value (rather than the
// default "*") is what gets reflected in the response.
static void reflects_configured_allowed_origin() {
    Cors defaultCors;
    Cors customCors("https://my-app.example");

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    NextHandler next = [](HttpRequest&, HttpResponse&) {};

    HttpResponse defaultResponse;
    defaultCors(request, defaultResponse, next);
    CHK(defaultResponse.header("access-control-allow-origin") == "*");

    HttpResponse customResponse;
    customCors(request, customResponse, next);
    CHK(customResponse.header("access-control-allow-origin") == "https://my-app.example");
}

// Executes all CORS preflight flow test cases.
static void run_tests() {
    RUN(options_request_short_circuits_with_204);
    RUN(non_options_request_gets_headers_and_continues);
    RUN(reflects_configured_allowed_origin);
}

REGISTER_TEST_SUITE();
