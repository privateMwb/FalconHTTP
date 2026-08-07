// Regression test: Router::dispatch() must return MethodNotAllowed
// (405) rather than NotFound (404) when a path matches a registered
// route but not for the request's method.
//
// The original bug: dispatch()'s loop only ever `continue`d past
// routes whose method didn't match, without checking whether the
// path itself matched - so a request to a real, registered path with
// the wrong method fell all the way through to the generic
// NotFound/404 case, and HttpStatus::MethodNotAllowed (405) was never
// actually reachable anywhere in the codebase. Fixed by tracking
// whether any non-matching-method route's pattern matched the path,
// and returning MethodNotAllowed for that case specifically.

#include <support/framework.h>

using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Routing;

// Verifies a request to a registered path with a method no route
// handles for that path returns MethodNotAllowed, not NotFound.
static void wrong_method_on_known_path_returns_method_not_allowed() {
    Router router;
    router.get("/items", [](const HttpRequest&, HttpResponse&) {});

    HttpRequest request;
    request.setMethod(HttpMethod::Post);
    request.setPath("/items");

    HttpResponse response;
    CHK(router.dispatch(request, response) == DispatchResult::MethodNotAllowed);
}

// Verifies a request to a path with no registered route at all still
// correctly returns NotFound, not MethodNotAllowed - the fix must not
// have over-corrected to always report MethodNotAllowed.
static void unknown_path_still_returns_not_found() {
    Router router;
    router.get("/items", [](const HttpRequest&, HttpResponse&) {});

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/does-not-exist");

    HttpResponse response;
    CHK(router.dispatch(request, response) == DispatchResult::NotFound);
}

// Verifies a path registered under multiple methods correctly returns
// MethodNotAllowed for a third, unregistered method - not just a
// simple single-route check.
static void method_not_allowed_with_multiple_registered_methods() {
    Router router;
    router.get("/items", [](const HttpRequest&, HttpResponse&) {});
    router.post("/items", [](const HttpRequest&, HttpResponse&) {});

    HttpRequest request;
    request.setMethod(HttpMethod::Put);
    request.setPath("/items");

    HttpResponse response;
    CHK(router.dispatch(request, response) == DispatchResult::MethodNotAllowed);
}

// Executes all method-not-allowed regression test cases.
static void run_tests() {
    RUN(wrong_method_on_known_path_returns_method_not_allowed);
    RUN(unknown_path_still_returns_not_found);
    RUN(method_not_allowed_with_multiple_registered_methods);
}

REGISTER_TEST_SUITE();
