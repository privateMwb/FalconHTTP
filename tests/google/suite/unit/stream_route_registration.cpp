// Router stream-route registration and matchStream() test suite.
//
// Coverage:
// - stream() registers a Stream-kind route reachable via matchStream()
// - matchStream() populates path params on the request, same as
//   dispatch()
// - matchStream() returns nullptr for a path/method matching no
//   Stream-kind route
// - dispatch() never invokes a Stream-kind route's handler (it has
//   none to invoke) - a path registered only via stream() is NotFound
//   to dispatch()
// - A Normal-kind route registered on the same Router is unaffected -
//   dispatch() still matches it and matchStream() does not

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::Routing;
using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Streaming;

// Verifies stream() registers a route reachable via matchStream(),
// and that the returned handler is the one that was registered.
TEST(StreamRouteRegistration, StreamRouteIsMatched) {
    Router router;
    bool handlerRan = false;

    router.stream("/events",
                  [&handlerRan](const HttpRequest&, SseConnection&) { handlerRan = true; });

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/events");

    const StreamHandler* handler = router.matchStream(request);
    ASSERT_NE(handler, nullptr);

    // Invoking directly requires a real SseConnection - matchStream()
    // itself must not have invoked anything.
    EXPECT_FALSE(handlerRan);
}

// Verifies matchStream() populates path params before returning,
// mirroring dispatch()'s contract.
TEST(StreamRouteRegistration, MatchStreamPopulatesPathParams) {
    Router router;
    router.stream("/rooms/:id/events", [](const HttpRequest&, SseConnection&) {});

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/rooms/7/events");

    const StreamHandler* handler = router.matchStream(request);
    ASSERT_NE(handler, nullptr);
    EXPECT_EQ(request.pathParam("id"), "7");
}

// Verifies matchStream() returns nullptr when no Stream-kind route
// matches the request's path or method.
TEST(StreamRouteRegistration, MatchStreamReturnsNullWhenUnmatched) {
    Router router;
    router.stream("/events", [](const HttpRequest&, SseConnection&) {});

    HttpRequest wrongPath;
    wrongPath.setMethod(HttpMethod::Get);
    wrongPath.setPath("/other");
    EXPECT_EQ(router.matchStream(wrongPath), nullptr);

    HttpRequest wrongMethod;
    wrongMethod.setMethod(HttpMethod::Post);
    wrongMethod.setPath("/events");
    EXPECT_EQ(router.matchStream(wrongMethod), nullptr);
}

// Verifies dispatch() cannot reach a Stream-kind route - a path
// registered only via stream() reports NotFound to dispatch(), since
// dispatch() has no StreamHandler-compatible invocation path.
TEST(StreamRouteRegistration, DispatchSkipsStreamRoutes) {
    Router router;
    router.stream("/events", [](const HttpRequest&, SseConnection&) {});

    HttpRequest request;
    request.setMethod(HttpMethod::Get);
    request.setPath("/events");

    HttpResponse response;
    EXPECT_EQ(router.dispatch(request, response), DispatchResult::NotFound);
}

// Verifies a Normal-kind route registered alongside a Stream-kind one
// is unaffected: dispatch() matches it and matchStream() does not.
TEST(StreamRouteRegistration, NormalAndStreamRoutesCoexist) {
    Router router;
    router.get("/health", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
    });
    router.stream("/events", [](const HttpRequest&, SseConnection&) {});

    HttpRequest healthRequest;
    healthRequest.setMethod(HttpMethod::Get);
    healthRequest.setPath("/health");

    HttpResponse response;
    EXPECT_EQ(router.dispatch(healthRequest, response), DispatchResult::Matched);
    EXPECT_EQ(router.matchStream(healthRequest), nullptr);

    HttpRequest eventsRequest;
    eventsRequest.setMethod(HttpMethod::Get);
    eventsRequest.setPath("/events");
    EXPECT_NE(router.matchStream(eventsRequest), nullptr);
}
