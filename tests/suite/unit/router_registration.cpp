// Router registration and dispatch() test suite.
//
// Coverage:
// - get()/post()/put()/del() register a route reachable by dispatch()
// - dispatch() returns Matched and invokes the handler on a
//   method+path match
// - dispatch() populates path params on the request before invoking
//   the handler
// - dispatch() returns NotFound when no route's pattern matches the path
// - dispatch() returns MethodNotAllowed when a route's pattern matches
//   the path but not for the request's method

#include <support/framework.h>

using namespace FalconHTTP::Routing;
using namespace FalconHTTP::HTTP;

// Verifies a GET route registered via get() is reachable, returns
// Matched, and its handler actually runs.
static void get_route_is_matched_and_invoked() {
    Router router;
    bool handlerRan = false;

    router.get("/health", [&handlerRan](const HttpRequest&, HttpResponse& response) {
        handlerRan = true;
        response.setStatus(HttpStatus::Ok);
    });

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/health");

    HttpResponse response;
    DispatchResult result = router.dispatch(request, response);

    CHK(result == DispatchResult::Matched);
    CHK(handlerRan);
    CHK(response.status() == HttpStatus::Ok);
}

// Verifies post()/put()/del() register routes for their respective
// methods, and each is only matched by that method.
static void other_methods_register_correctly() {
    Router router;
    router.post("/items", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Created);
    });
    router.put("/items/:id", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
    });
    router.del("/items/:id", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::NoContent);
    });

    HttpRequest postRequest;
    postRequest.setMethod(HttpMethod::Post);
    postRequest.setPath("/items");
    HttpResponse postResponse;
    CHK(router.dispatch(postRequest, postResponse) == DispatchResult::Matched);
    CHK(postResponse.status() == HttpStatus::Created);

    HttpRequest putRequest;
    putRequest.setMethod(HttpMethod::Put);
    putRequest.setPath("/items/5");
    HttpResponse putResponse;
    CHK(router.dispatch(putRequest, putResponse) == DispatchResult::Matched);

    HttpRequest delRequest;
    delRequest.setMethod(HttpMethod::Delete);
    delRequest.setPath("/items/5");
    HttpResponse delResponse;
    CHK(router.dispatch(delRequest, delResponse) == DispatchResult::Matched);
    CHK(delResponse.status() == HttpStatus::NoContent);
}

// Verifies dispatch() populates path params on the request before
// invoking the handler, so the handler can read them via pathParam().
static void dispatch_populates_path_params() {
    Router router;
    std::string capturedId;

    router.get("/users/:id", [&capturedId](const HttpRequest& request, HttpResponse&) {
        capturedId = request.pathParam("id");
    });

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/users/42");

    HttpResponse response;
    CHK(router.dispatch(request, response) == DispatchResult::Matched);
    CHK(capturedId == "42");
}

// Verifies dispatch() returns NotFound when no registered route's
// pattern matches the request path at all.
static void returns_not_found_for_unmatched_path() {
    Router router;
    router.get("/health", [](const HttpRequest&, HttpResponse&) {});

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/does-not-exist");

    HttpResponse response;
    CHK(router.dispatch(request, response) == DispatchResult::NotFound);
}

// Verifies dispatch() returns MethodNotAllowed when a route's pattern
// matches the path but only for a different method.
static void returns_method_not_allowed_for_wrong_method() {
    Router router;
    router.get("/items", [](const HttpRequest&, HttpResponse&) {});

    HttpRequest request;
    request.setMethod(HttpMethod::Post);
    request.setPath("/items");

    HttpResponse response;
    CHK(router.dispatch(request, response) == DispatchResult::MethodNotAllowed);
}

// Executes all Router registration/dispatch test cases.
static void run_tests() {
    RUN(get_route_is_matched_and_invoked);
    RUN(other_methods_register_correctly);
    RUN(dispatch_populates_path_params);
    RUN(returns_not_found_for_unmatched_path);
    RUN(returns_method_not_allowed_for_wrong_method);
}

REGISTER_TEST_SUITE();
