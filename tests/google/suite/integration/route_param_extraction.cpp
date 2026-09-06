// Route parameter extraction integration test suite: HttpParser ->
// PathMatcher (via Router::dispatch()) -> handler.
//
// Coverage:
// - A path param parsed from a raw request reaches the handler via
//   HttpRequest::pathParam()
// - A query param parsed (and percent-decoded) from the same raw
//   request reaches the handler via HttpRequest::queryParam(), fully
//   decoded
// - Multiple path params in one pattern all bind correctly when
//   driven through the real parser, not just constructed by hand
//
// NOTE: this differs from unit/router_registration.cpp's param test,
// which builds the HttpRequest by hand (setPath/setMethod) rather
// than parsing raw bytes - this suite verifies the parser's path/query
// split and PathMatcher's binding work correctly together, not just
// each in isolation.

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Routing;

// Verifies a single path param parsed from a raw request reaches the
// handler correctly.
TEST(RouteParamExtraction, SinglePathParamReachesHandler) {
    Router router;
    std::string captured;

    router.get("/users/:id", [&captured](const HttpRequest& request, HttpResponse&) {
        captured = request.pathParam("id");
    });

    HttpRequest request = HttpParser::parse("GET /users/77 HTTP/1.1\r\nHost: h\r\n\r\n");
    HttpResponse response;
    (void)router.dispatch(request, response);

    EXPECT_EQ(captured, "77");
}

// Verifies multiple path params in one pattern all bind correctly
// when the request is driven through the real parser.
TEST(RouteParamExtraction, MultiplePathParamsReachHandler) {
    Router router;
    std::string capturedUser;
    std::string capturedPost;

    router.get("/users/:userId/posts/:postId", [&](const HttpRequest& request, HttpResponse&) {
        capturedUser = request.pathParam("userId");
        capturedPost = request.pathParam("postId");
    });

    HttpRequest request = HttpParser::parse("GET /users/3/posts/91 HTTP/1.1\r\nHost: h\r\n\r\n");
    HttpResponse response;
    (void)router.dispatch(request, response);

    EXPECT_EQ(capturedUser, "3");
    EXPECT_EQ(capturedPost, "91");
}

// Verifies a percent-encoded query parameter is decoded before
// reaching the handler.
TEST(RouteParamExtraction, QueryParamIsDecodedBeforeReachingHandler) {
    Router router;
    std::string captured;

    router.get("/search", [&captured](const HttpRequest& request, HttpResponse&) {
        captured = request.queryParam("q");
    });

    HttpRequest request =
        HttpParser::parse("GET /search?q=hello%20world HTTP/1.1\r\nHost: h\r\n\r\n");
    HttpResponse response;
    (void)router.dispatch(request, response);

    EXPECT_EQ(captured, "hello world");
}

// Verifies a path param and a query param on the same request both
// reach the handler correctly at once.
TEST(RouteParamExtraction, PathAndQueryParamsCoexist) {
    Router router;
    std::string capturedId;
    std::string capturedSort;

    router.get("/items/:id", [&](const HttpRequest& request, HttpResponse&) {
        capturedId = request.pathParam("id");
        capturedSort = request.queryParam("sort");
    });

    HttpRequest request = HttpParser::parse("GET /items/9?sort=desc HTTP/1.1\r\nHost: h\r\n\r\n");
    HttpResponse response;
    (void)router.dispatch(request, response);

    EXPECT_EQ(capturedId, "9");
    EXPECT_EQ(capturedSort, "desc");
}
