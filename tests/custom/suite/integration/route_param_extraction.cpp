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

#include <support/framework.h>

using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Routing;

// Verifies a single path param parsed from a raw request reaches the
// handler correctly.
static void single_path_param_reaches_handler() {
    Router router;
    std::string captured;

    router.get("/users/:id", [&captured](const HttpRequest& request, HttpResponse&) {
        captured = request.pathParam("id");
    });

    HttpRequest request = HttpParser::parse("GET /users/77 HTTP/1.1\r\nHost: h\r\n\r\n");
    HttpResponse response;
    (void)router.dispatch(request, response);

    CHK(captured == "77");
}

// Verifies multiple path params in one pattern all bind correctly
// when the request is driven through the real parser.
static void multiple_path_params_reach_handler() {
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

    CHK(capturedUser == "3");
    CHK(capturedPost == "91");
}

// Verifies a percent-encoded query parameter is decoded before
// reaching the handler.
static void query_param_is_decoded_before_reaching_handler() {
    Router router;
    std::string captured;

    router.get("/search", [&captured](const HttpRequest& request, HttpResponse&) {
        captured = request.queryParam("q");
    });

    HttpRequest request =
        HttpParser::parse("GET /search?q=hello%20world HTTP/1.1\r\nHost: h\r\n\r\n");
    HttpResponse response;
    (void)router.dispatch(request, response);

    CHK(captured == "hello world");
}

// Verifies a path param and a query param on the same request both
// reach the handler correctly at once.
static void path_and_query_params_coexist() {
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

    CHK(capturedId == "9");
    CHK(capturedSort == "desc");
}

// Executes all route parameter extraction test cases.
static void run_tests() {
    RUN(single_path_param_reaches_handler);
    RUN(multiple_path_params_reach_handler);
    RUN(query_param_is_decoded_before_reaching_handler);
    RUN(path_and_query_params_coexist);
}

REGISTER_TEST_SUITE();
