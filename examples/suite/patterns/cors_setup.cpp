// Configuring CORS.
//
// Demonstrates:
// - Cors's default constructor (allows all origins)
// - Restricting to a specific origin
// - The Access-Control-Allow-* headers added to a normal request
// - OPTIONS preflight requests being short-circuited with 204 No Content

#include <support/framework.h>

#include <iostream>

using namespace FalconHTTP;

static void run_examples() {

    // Cors() with no argument allows any origin ("*"). It always calls
    // next() for non-OPTIONS requests, adding headers on the way out.
    setTitle("Default: Allow All Origins");

    Middleware::Cors cors;

    HTTP::HttpRequest request;
    request.setMethod(HTTP::HttpMethod::Get);

    HTTP::HttpResponse response;
    Middleware::NextHandler markHandled([](HTTP::HttpRequest&, HTTP::HttpResponse& response) {
        response.setStatus(HTTP::HttpStatus::Ok);
        response.setBody("handled");
    });

    cors(request, response, markHandled);

    std::cout << "allow-origin  : " << response.header("Access-Control-Allow-Origin") << "\n";
    std::cout << "allow-methods : " << response.header("Access-Control-Allow-Methods") << "\n";
    std::cout << "allow-headers : " << response.header("Access-Control-Allow-Headers") << "\n";
    std::cout << "status        : " << static_cast<int>(response.status()) << "\n\n";

    // Passing an explicit origin restricts Access-Control-Allow-Origin to
    // just that value instead of "*".
    setTitle("Restricting to One Origin");

    Middleware::Cors restrictedCors("https://example.com");

    HTTP::HttpRequest restrictedRequest;
    restrictedRequest.setMethod(HTTP::HttpMethod::Get);

    HTTP::HttpResponse restrictedResponse;
    restrictedCors(restrictedRequest, restrictedResponse, markHandled);

    std::cout << "allow-origin : " << restrictedResponse.header("Access-Control-Allow-Origin")
              << "\n\n";

    // An OPTIONS request is treated as a preflight check: Cors responds
    // 204 No Content itself and does NOT call next() - the rest of the
    // chain and the route handler never run.
    setTitle("OPTIONS Preflight");

    HTTP::HttpRequest preflightRequest;
    preflightRequest.setMethod(HTTP::HttpMethod::Options);

    HTTP::HttpResponse preflightResponse;
    bool nextWasCalled = false;
    Middleware::NextHandler trackNext(
        [&nextWasCalled](HTTP::HttpRequest&, HTTP::HttpResponse&) { nextWasCalled = true; });

    cors(preflightRequest, preflightResponse, trackNext);

    std::cout << "status         : " << static_cast<int>(preflightResponse.status()) << "\n";
    std::cout << "next() called  : " << nextWasCalled << "\n";
}

REGISTER_EXAMPLE_SUITE();
