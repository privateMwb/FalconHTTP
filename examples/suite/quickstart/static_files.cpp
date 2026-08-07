// Serving static files.
//
// Demonstrates:
// - Constructing a StaticFileServer over a root directory
// - Serving an existing file, with its Content-Type inferred from extension
// - A miss on a file that doesn't exist
// - Wiring StaticFileServer into a Router route (single-segment paths only -
//   see PathMatcher, there is no wildcard/catch-all segment support)

#include <support/framework.h>

#include <filesystem>
#include <fstream>
#include <iostream>

using namespace FalconHTTP;

static void run_examples() {

    // StaticFileServer resolves a request path against a fixed root
    // directory, with an in-memory LRU cache (FileCache) for served content.
    setTitle("Setup");

    std::filesystem::path root =
        std::filesystem::temp_directory_path() / "falconhttp_static_example";
    std::filesystem::create_directories(root);

    std::ofstream(root / "index.html") << "<h1>Hello, static file!</h1>";

    FileServer::StaticFileServer staticFiles(root.string(), /*cacheCapacity=*/64);

    std::cout << "root directory : " << root.string() << "\n\n";

    // serve() populates the response (status 200, Content-Type, body) and
    // returns true on success; the response is left untouched on failure.
    setTitle("Serving an Existing File");

    HTTP::HttpResponse response;
    bool found = staticFiles.serve("/index.html", response);

    std::cout << "found        : " << found << "\n";
    std::cout << "status       : " << static_cast<int>(response.status()) << "\n";
    std::cout << "content-type : " << response.header("Content-Type") << "\n";
    std::cout << "body         : " << response.body() << "\n\n";

    // A missing file returns false; the caller (see Server::runChain() or a
    // route handler) is responsible for turning that into a 404 response.
    setTitle("A Missing File");

    HTTP::HttpResponse missingResponse;
    bool missingFound = staticFiles.serve("/does-not-exist.html", missingResponse);

    std::cout << "found : " << missingFound << "\n\n";

    // Wiring it into a Router: since PathMatcher has no wildcard segment,
    // a single ":file" param only reaches files directly under the root -
    // it won't match nested paths like "/static/css/site.css".
    setTitle("Wiring Into a Router");

    Routing::Router router;

    router.get("/static/:file",
               [&staticFiles](const HTTP::HttpRequest& request, HTTP::HttpResponse& response) {
                   std::string file = request.pathParam("file");

                   if (!staticFiles.serve("/" + file, response)) {
                       response.setStatus(HTTP::HttpStatus::NotFound);
                       response.setBody("File not found");
                   }
               });

    HTTP::HttpRequest request;
    request.setMethod(HTTP::HttpMethod::Get);
    request.setPath("/static/index.html");

    HTTP::HttpResponse routedResponse;
    (void)router.dispatch(request, routedResponse);

    std::cout << "status : " << static_cast<int>(routedResponse.status()) << "\n";
    std::cout << "body   : " << routedResponse.body() << "\n";

    std::filesystem::remove_all(root);
}

REGISTER_EXAMPLE_SUITE();
