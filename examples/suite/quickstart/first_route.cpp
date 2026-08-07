// Registering your first route.
//
// Demonstrates:
// - Constructing a Router and registering a GET handler
// - Reading a path parameter inside a handler
// - Dispatching a request directly against the Router (no live server needed)
// - Telling a matched request apart from an unmatched one via DispatchResult

#include <support/framework.h>

#include <iostream>

using namespace FalconHTTP;

static void run_examples() {

    // A Router holds registered routes and matches an incoming request's
    // method and path against them, in registration order. A `:name`
    // segment matches any single path segment and binds it as a path param.
    setTitle("Registering a Route");

    Routing::Router router;

    router.get("/users/:id", [](const HTTP::HttpRequest& request, HTTP::HttpResponse& response) {
        std::string id = request.pathParam("id");

        response.setStatus(HTTP::HttpStatus::Ok);
        response.setBody("User " + id);
    });

    std::cout << "route registered: GET /users/:id\n\n";

    // dispatch() finds the first route whose method and path pattern both
    // match, populates path params on the request, and invokes its handler.
    setTitle("Dispatching a Matching Request");

    HTTP::HttpRequest request;
    request.setMethod(HTTP::HttpMethod::Get);
    request.setPath("/users/42");

    HTTP::HttpResponse response;
    Routing::DispatchResult result = router.dispatch(request, response);

    std::cout << "matched : " << (result == Routing::DispatchResult::Matched) << "\n";
    std::cout << "status  : " << static_cast<int>(response.status()) << "\n";
    std::cout << "body    : " << response.body() << "\n\n";

    // A path that matches no registered pattern reports NotFound, and the
    // response is left untouched - it's the caller's job (see Server::runChain())
    // to turn that into an actual 404.
    setTitle("Dispatching a Non-Matching Request");

    HTTP::HttpRequest missingRequest;
    missingRequest.setMethod(HTTP::HttpMethod::Get);
    missingRequest.setPath("/unknown");

    HTTP::HttpResponse missingResponse;
    Routing::DispatchResult missingResult = router.dispatch(missingRequest, missingResponse);

    std::cout << "matched : " << (missingResult == Routing::DispatchResult::Matched) << "\n";
    std::cout << "result  : "
              << (missingResult == Routing::DispatchResult::NotFound ? "NotFound" : "other")
              << "\n";
}

REGISTER_EXAMPLE_SUITE();
