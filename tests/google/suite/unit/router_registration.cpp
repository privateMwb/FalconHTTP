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

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::Routing;
using namespace FalconHTTP::HTTP;

// Verifies a GET route registered via get() is reachable, returns
// Matched, and its handler actually runs.
TEST(RouterRegistration, GetRouteIsMatchedAndInvoked) {
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

    EXPECT_EQ(result, DispatchResult::Matched);
    EXPECT_TRUE(handlerRan);
    EXPECT_EQ(response.status(), HttpStatus::Ok);
}

// Verifies post()/put()/del() register routes for their respective
// methods, and each is only matched by that method.
TEST(RouterRegistration, OtherMethodsRegisterCorrectly) {
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
    ASSERT_EQ(router.dispatch(postRequest, postResponse), DispatchResult::Matched);
    EXPECT_EQ(postResponse.status(), HttpStatus::Created);

    HttpRequest putRequest;
    putRequest.setMethod(HttpMethod::Put);
    putRequest.setPath("/items/5");
    HttpResponse putResponse;
    EXPECT_EQ(router.dispatch(putRequest, putResponse), DispatchResult::Matched);

    HttpRequest delRequest;
    delRequest.setMethod(HttpMethod::Delete);
    delRequest.setPath("/items/5");
    HttpResponse delResponse;
    ASSERT_EQ(router.dispatch(delRequest, delResponse), DispatchResult::Matched);
    EXPECT_EQ(delResponse.status(), HttpStatus::NoContent);
}

// Verifies dispatch() populates path params on the request before
// invoking the handler, so the handler can read them via pathParam().
TEST(RouterRegistration, DispatchPopulatesPathParams) {
    Router router;
    std::string capturedId;

    router.get("/users/:id", [&capturedId](const HttpRequest& request, HttpResponse&) {
        capturedId = request.pathParam("id");
    });

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/users/42");

    HttpResponse response;
    ASSERT_EQ(router.dispatch(request, response), DispatchResult::Matched);
    EXPECT_EQ(capturedId, "42");
}

// Verifies dispatch() returns NotFound when no registered route's
// pattern matches the request path at all.
TEST(RouterRegistration, ReturnsNotFoundForUnmatchedPath) {
    Router router;
    router.get("/health", [](const HttpRequest&, HttpResponse&) {});

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/does-not-exist");

    HttpResponse response;
    EXPECT_EQ(router.dispatch(request, response), DispatchResult::NotFound);
}

// Verifies dispatch() returns MethodNotAllowed when a route's pattern
// matches the path but only for a different method.
TEST(RouterRegistration, ReturnsMethodNotAllowedForWrongMethod) {
    Router router;
    router.get("/items", [](const HttpRequest&, HttpResponse&) {});

    HttpRequest request;
    request.setMethod(HttpMethod::Post);
    request.setPath("/items");

    HttpResponse response;
    EXPECT_EQ(router.dispatch(request, response), DispatchResult::MethodNotAllowed);
}
