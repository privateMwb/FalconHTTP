// Shaping an HttpResponse.
//
// Demonstrates:
// - Setting a status code and reading its reason phrase
// - Setting a plain-text body
// - Setting custom headers
// - Setting a JSON body via setJson()

#include <support/framework.h>

#include <iostream>

using namespace FalconHTTP;

static void run_examples() {

    // A default-constructed HttpResponse has no status set until you call
    // setStatus() - handlers are expected to always set one explicitly.
    setTitle("Status and Body");

    HTTP::HttpResponse response;
    response.setStatus(HTTP::HttpStatus::Ok);
    response.setBody("Hello from FalconHTTP!");

    std::cout << "status : " << static_cast<int>(response.status()) << "\n";
    std::cout << "body   : " << response.body() << "\n\n";

    // Headers are set individually; hasHeader()/header() read them back.
    setTitle("Custom Headers");

    response.setHeader("X-Powered-By", "FalconHTTP");

    std::cout << "has header : " << response.hasHeader("X-Powered-By") << "\n";
    std::cout << "value      : " << response.header("X-Powered-By") << "\n\n";

    // setJson() serializes a Json value into the body and sets the
    // Content-Type header for you.
    setTitle("JSON Body");

    Json::ObjectType obj;
    obj["id"] = Json("42");
    obj["message"] = Json("User lookup placeholder");

    HTTP::HttpResponse jsonResponse;
    jsonResponse.setStatus(HTTP::HttpStatus::Ok);
    jsonResponse.setJson(Json(std::move(obj)));

    std::cout << "content-type : " << jsonResponse.header("Content-Type") << "\n";
    std::cout << "body         : " << jsonResponse.body() << "\n\n";

    // An error response looks the same as a success response - only the
    // status code and body content signal that something went wrong.
    setTitle("Error Response");

    HTTP::HttpResponse notFound(HTTP::HttpStatus::NotFound);
    notFound.setBody("No such user");

    std::cout << "status : " << static_cast<int>(notFound.status()) << "\n";
    std::cout << "body   : " << notFound.body() << "\n";
}

REGISTER_EXAMPLE_SUITE();
