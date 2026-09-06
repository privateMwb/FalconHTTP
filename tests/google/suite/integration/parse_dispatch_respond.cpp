// Full request pipeline integration test suite: HttpParser -> Router
// -> HttpSerializer.
//
// Coverage:
// - A raw GET request with a path param and query string parses,
//   dispatches to the correct handler, and serializes to the expected
//   response
// - A raw POST request's body survives the full pipeline unchanged
// - A request for an unregistered path serializes to a 404 response
//   (Router::dispatch() returning NotFound, handled by the same logic
//   Server::runChain() uses)

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Routing;

namespace {

// Mirrors the 404 handling Server::runChain() applies after
// Router::dispatch() - see Server.cpp. Kept local since runChain() is
// private; this exercises the same DispatchResult contract directly.
void applyDispatchResult(DispatchResult result, HttpResponse& response) {
    if (result == DispatchResult::NotFound) {
        response.setStatus(HttpStatus::NotFound);
        response.setBody("Not Found");
    } else if (result == DispatchResult::MethodNotAllowed) {
        response.setStatus(HttpStatus::MethodNotAllowed);
        response.setBody("Method Not Allowed");
    }
}

} // namespace

// Verifies a GET request with a path param and query string parses,
// reaches the correct handler with both available, and serializes
// correctly.
TEST(ParseDispatchRespond, GetRequestRoundTripsThroughPipeline) {
    Router router;
    router.get("/users/:id", [](const HttpRequest& request, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
        response.setBody("id=" + request.pathParam("id") +
                         " verbose=" + request.queryParam("verbose"));
    });

    std::string raw = "GET /users/42?verbose=true HTTP/1.1\r\n"
                      "Host: example.com\r\n"
                      "\r\n";

    HttpRequest request = HttpParser::parse(raw);
    HttpResponse response;

    applyDispatchResult(router.dispatch(request, response), response);

    std::string output = HttpSerializer::serialize(response);

    EXPECT_TRUE(output.starts_with("HTTP/1.1 200 OK\r\n"));
    EXPECT_TRUE(output.ends_with("id=42 verbose=true"));
}

// Verifies a POST request's body survives HttpParser::parse() and
// reaches the handler unchanged.
TEST(ParseDispatchRespond, PostRequestBodySurvivesPipeline) {
    Router router;
    router.post("/echo", [](const HttpRequest& request, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
        response.setBody(request.body());
    });

    std::string raw = "POST /echo HTTP/1.1\r\n"
                      "Content-Type: text/plain\r\n"
                      "Content-Length: 5\r\n"
                      "\r\n"
                      "hello";

    HttpRequest request = HttpParser::parse(raw);
    HttpResponse response;

    applyDispatchResult(router.dispatch(request, response), response);

    std::string output = HttpSerializer::serialize(response);

    EXPECT_TRUE(output.ends_with("hello"));
}

// Verifies a request for a path with no matching route serializes to
// a 404 response through the same DispatchResult path Server uses.
TEST(ParseDispatchRespond, UnmatchedPathSerializesToNotFound) {
    Router router;
    router.get("/known", [](const HttpRequest&, HttpResponse&) {});

    std::string raw = "GET /unknown HTTP/1.1\r\nHost: example.com\r\n\r\n";

    HttpRequest request = HttpParser::parse(raw);
    HttpResponse response;

    applyDispatchResult(router.dispatch(request, response), response);

    std::string output = HttpSerializer::serialize(response);

    EXPECT_TRUE(output.starts_with("HTTP/1.1 404 Not Found\r\n"));
}
