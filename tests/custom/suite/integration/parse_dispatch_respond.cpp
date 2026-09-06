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

#include <support/framework.h>

using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Routing;

// Mirrors the 404 handling Server::runChain() applies after
// Router::dispatch() - see Server.cpp. Kept local since runChain() is
// private; this exercises the same DispatchResult contract directly.
static void applyDispatchResult(DispatchResult result, HttpResponse& response) {
    if (result == DispatchResult::NotFound) {
        response.setStatus(HttpStatus::NotFound);
        response.setBody("Not Found");
    } else if (result == DispatchResult::MethodNotAllowed) {
        response.setStatus(HttpStatus::MethodNotAllowed);
        response.setBody("Method Not Allowed");
    }
}

// Verifies a GET request with a path param and query string parses,
// reaches the correct handler with both available, and serializes
// correctly.
static void get_request_round_trips_through_pipeline() {
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

    CHK(output.starts_with("HTTP/1.1 200 OK\r\n"));
    CHK(output.ends_with("id=42 verbose=true"));
}

// Verifies a POST request's body survives HttpParser::parse() and
// reaches the handler unchanged.
static void post_request_body_survives_pipeline() {
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

    CHK(output.ends_with("hello"));
}

// Verifies a request for a path with no matching route serializes to
// a 404 response through the same DispatchResult path Server uses.
static void unmatched_path_serializes_to_not_found() {
    Router router;
    router.get("/known", [](const HttpRequest&, HttpResponse&) {});

    std::string raw = "GET /unknown HTTP/1.1\r\nHost: example.com\r\n\r\n";

    HttpRequest request = HttpParser::parse(raw);
    HttpResponse response;

    applyDispatchResult(router.dispatch(request, response), response);

    std::string output = HttpSerializer::serialize(response);

    CHK(output.starts_with("HTTP/1.1 404 Not Found\r\n"));
}

// Executes all parse-dispatch-respond pipeline test cases.
static void run_tests() {
    RUN(get_request_round_trips_through_pipeline);
    RUN(post_request_body_survives_pipeline);
    RUN(unmatched_path_serializes_to_not_found);
}

REGISTER_TEST_SUITE();
