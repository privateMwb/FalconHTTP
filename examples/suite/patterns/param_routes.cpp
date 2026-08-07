// Path params and query params.
//
// Demonstrates:
// - A pattern with more than one :param segment
// - Reading path params vs query params inside a handler
// - hasPathParam()/hasQueryParam() for optional values
// - Path params are strings - a handler is responsible for converting them

#include <support/framework.h>

#include <iostream>

using namespace FalconHTTP;

static void run_examples() {

    // Each `:name` segment in a pattern binds the corresponding path
    // segment under that name - "/posts/:postId/comments/:commentId"
    // requires both segments to be present to match at all.
    setTitle("Multiple Path Params");

    Routing::Router router;

    router.get("/posts/:postId/comments/:commentId",
               [](const HTTP::HttpRequest& request, HTTP::HttpResponse& response) {
                   std::string postId = request.pathParam("postId");
                   std::string commentId = request.pathParam("commentId");

                   response.setStatus(HTTP::HttpStatus::Ok);
                   response.setBody("post " + postId + ", comment " + commentId);
               });

    HTTP::HttpRequest request;
    request.setMethod(HTTP::HttpMethod::Get);
    request.setPath("/posts/7/comments/3");

    HTTP::HttpResponse response;
    (void)router.dispatch(request, response);

    std::cout << "body : " << response.body() << "\n\n";

    // Query params live separately from path params - they're populated by
    // whatever parses the request (see HttpParser), not by the Router, and
    // a route pattern says nothing about which ones it expects.
    setTitle("Query Params");

    HTTP::HttpRequest searchRequest;
    searchRequest.setMethod(HTTP::HttpMethod::Get);
    searchRequest.setPath("/posts/7/comments/3");
    searchRequest.setQueryParam("sort", "newest");

    std::cout << "has 'sort' query param : " << searchRequest.hasQueryParam("sort") << "\n";
    std::cout << "sort                   : " << searchRequest.queryParam("sort") << "\n";
    std::cout << "has 'page' query param : " << searchRequest.hasQueryParam("page") << "\n\n";

    // Both path and query params come back as plain strings - converting
    // to a number (or validating the format) is left to the handler.
    setTitle("Converting a Param");

    router.get("/items/:itemId",
               [](const HTTP::HttpRequest& request, HTTP::HttpResponse& response) {
                   std::string itemIdText = request.pathParam("itemId");

                   try {
                       int itemId = std::stoi(itemIdText);
                       response.setStatus(HTTP::HttpStatus::Ok);
                       response.setBody("item #" + std::to_string(itemId));
                   } catch (const std::exception&) {
                       response.setStatus(HTTP::HttpStatus::BadRequest);
                       response.setBody("itemId must be numeric");
                   }
               });

    HTTP::HttpRequest itemRequest;
    itemRequest.setMethod(HTTP::HttpMethod::Get);
    itemRequest.setPath("/items/not-a-number");

    HTTP::HttpResponse itemResponse;
    (void)router.dispatch(itemRequest, itemResponse);

    std::cout << "status : " << static_cast<int>(itemResponse.status()) << "\n";
    std::cout << "body   : " << itemResponse.body() << "\n";
}

REGISTER_EXAMPLE_SUITE();
