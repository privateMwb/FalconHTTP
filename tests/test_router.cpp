// Router Test Suite
// Validates HTTP routing behavior including route registration, dispatching,
// method matching, path parameters, and route precedence rules.
//
// Covers:
// - simple route registration and dispatch
// - unmatched routes returning false
// - HTTP method mismatches
// - path parameter extraction in handlers
// - route priority (first match wins)

#include "Routing/Router.h"
#include "HTTP/HttpRequest.h"
#include "HTTP/HttpResponse.h"

#include "test_helper.h"

using namespace FalconHTTP::Routing;
using namespace FalconHTTP::HTTP;

// Shared Router instance used across tests
static Router router;

// Simple route registration and successful dispatch test
// Ensures handler is executed, response status is set, and dispatch returns true
static void register_and_match_simple_route() {
    bool handlerRan = false;

    router.get("/home", [&handlerRan](const HttpRequest&, HttpResponse& response) {
        handlerRan = true;
        response.setStatus(HttpStatus::Ok);
        });

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/home");

    HttpResponse response;
    bool matched = router.dispatch(request, response);

    CHK(matched == true);
    CHK(handlerRan == true);
    CHK(response.status() == HttpStatus::Ok);
}

// No matching route test
// Ensures dispatch returns false when route is not registered
static void no_match_returns_false() {
    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/does-not-exist");

    HttpResponse response;
    bool matched = router.dispatch(request, response);

    CHK(matched == false);
}

// HTTP method mismatch test
// Ensures route does not match when HTTP method differs
static void method_mismatch_returns_false() {
    HttpRequest request;
    request.setMethod(HttpMethod::Post);
    request.setPath("/home");

    HttpResponse response;
    bool matched = router.dispatch(request, response);

    CHK(matched == false);
}

// Path parameter dispatch test
// Ensures route parameters are extracted and accessible inside handler
static void path_param_dispatch() {
    router.get("/users/:id", [](const HttpRequest& request, HttpResponse& response) {
        response.setBody(request.pathParam("id"));
        response.setStatus(HttpStatus::Ok);
        });

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/users/55");

    HttpResponse response;
    bool matched = router.dispatch(request, response);

    CHK(matched == true);
    CHK(response.body() == "55");
}

// Multiple route priority test
// Ensures that the first registered matching route is executed
static void multiple_routes_first_match_wins() {
    Router localRouter;
    int calledHandler = 0;

    localRouter.get("/items/:id", [&calledHandler](const HttpRequest&, HttpResponse&) {
        calledHandler = 1;
    });

    localRouter.get("/items/special", [&calledHandler](const HttpRequest&, HttpResponse&) {
        calledHandler = 2;
    });

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/items/special");

    HttpResponse response;
    (void)localRouter.dispatch(request, response);

    CHK(calledHandler == 1);
}

// Test runner for Router
// Executes all routing-related test cases
void run_router_tests() {
    setTitle("Router Tests");
    
    RUN(register_and_match_simple_route);
    RUN(no_match_returns_false);
    RUN(method_mismatch_returns_false);
    RUN(path_param_dispatch);
    RUN(multiple_routes_first_match_wins);

    std::cout<< "\n";
}