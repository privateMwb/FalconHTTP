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

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Routing;

// Verifies a request to a registered path with a method no route
// handles for that path returns MethodNotAllowed, not NotFound.
TEST(MethodNotAllowedRouter, WrongMethodOnKnownPathReturnsMethodNotAllowed) {
    Router router;
    router.get("/items", [](const HttpRequest&, HttpResponse&) {});

    HttpRequest request;
    request.setMethod(HttpMethod::Post);
    request.setPath("/items");

    HttpResponse response;
    EXPECT_EQ(router.dispatch(request, response), DispatchResult::MethodNotAllowed);
}

// Verifies a request to a path with no registered route at all still
// correctly returns NotFound, not MethodNotAllowed - the fix must not
// have over-corrected to always report MethodNotAllowed.
TEST(MethodNotAllowedRouter, UnknownPathStillReturnsNotFound) {
    Router router;
    router.get("/items", [](const HttpRequest&, HttpResponse&) {});

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/does-not-exist");

    HttpResponse response;
    EXPECT_EQ(router.dispatch(request, response), DispatchResult::NotFound);
}

// Verifies a path registered under multiple methods correctly returns
// MethodNotAllowed for a third, unregistered method - not just a
// simple single-route check.
TEST(MethodNotAllowedRouter, MethodNotAllowedWithMultipleRegisteredMethods) {
    Router router;
    router.get("/items", [](const HttpRequest&, HttpResponse&) {});
    router.post("/items", [](const HttpRequest&, HttpResponse&) {});

    HttpRequest request;
    request.setMethod(HttpMethod::Put);
    request.setPath("/items");

    HttpResponse response;
    EXPECT_EQ(router.dispatch(request, response), DispatchResult::MethodNotAllowed);
}
