// StaticFileServer Test Suite
// Validates static file serving behavior including file lookup, MIME type detection,
// nested directory support, path traversal protection, and cache consistency.
//
// Covers:
// - serving existing files
// - MIME type detection
// - nested file serving
// - missing file handling
// - path traversal protection
// - cached file consistency

#include "FileServer/StaticFileServer.h"
#include "HTTP/HttpResponse.h"
#include "HTTP/HttpStatus.h"

#include "test_helper.h"

#include <filesystem>
#include <fstream>

using namespace FalconHTTP::FileServer;
using namespace FalconHTTP::HTTP;

// Shared test directory
static std::filesystem::path testRoot = "test/static_root";

// Test setup
// Creates the test directory structure and sample files
static void setup() {
    std::filesystem::create_directories(testRoot / "nested");

    {
        std::ofstream file(testRoot / "index.html");
        file << "<h1>Hello</h1>";
    }

    {
        std::ofstream file(testRoot / "styles.css");
        file << "body { color: red; }";
    }

    {
        std::ofstream file(testRoot / "nested" / "page.html");
        file << "<p>Nested</p>";
    }
}

// Test teardown
// Removes all temporary files and directories created for testing
static void teardown() {
    std::filesystem::remove_all(testRoot);
}

// Existing file serving test
// Ensures an existing file is served with the correct status, body, and MIME type
static void server_existing_file() {
    StaticFileServer server(testRoot.string(), 8);
    HttpResponse response;

    bool served = server.serve("/index.html", response);

    CHK(served == true);
    CHK(response.status() == HttpStatus::Ok);
    CHK(response.body() == "<h1>Hello</h1>");
    CHK(response.header("Content-Type") == "text/html");
}

// CSS MIME type detection test
// Ensures CSS files are served with the "text/css" content type
static void correct_mime_type_for_css() {
    StaticFileServer server(testRoot.string(), 8);
    HttpResponse response;

    (void)server.serve("/styles.css", response);

    CHK(response.header("Content-Type") == "text/css");
}

// Nested file serving test
// Ensures files inside subdirectories are served correctly
static void nested_path_served() {
    StaticFileServer server(testRoot.string(), 8);
    HttpResponse response;

    bool served = server.serve("/nested/page.html", response);

    CHK(served == true);
    CHK(response.body() == "<p>Nested</p>");
}

// Missing file test
// Ensures requests for nonexistent files return false
static void nonexistent_file_returns_false() {
    StaticFileServer server(testRoot.string(), 8);
    HttpResponse response;

    bool served = server.serve("/missing.html", response);

    CHK(served == false);
}

// Path traversal protection test
// Ensures attempts to escape the document root are rejected
static void path_traversal_blocked() {
    StaticFileServer server(testRoot.string(), 8);
    HttpResponse response;

    bool served = server.serve("/../../../etc/passwd", response);

    CHK(served == false);
}

// Cache consistency test
// Ensures repeated requests return identical cached content
static void cache_returns_same_content_on_second_request() {
    StaticFileServer server(testRoot.string(), 8);

    HttpResponse first;
    HttpResponse second;

    (void)server.serve("/index.html", first);
    (void)server.serve("/index.html", second);

    CHK(first.body() == second.body());
    CHK(second.status() == HttpStatus::Ok);
}

// Test runner for StaticFileServer
// Executes all static file server test cases
void run_static_file_server_tests() {
    setTitle("StaticFileServer Tests");

    setup();

    RUN(server_existing_file);
    RUN(correct_mime_type_for_css);
    RUN(nested_path_served);
    RUN(nonexistent_file_returns_false);
    RUN(path_traversal_blocked);
    RUN(cache_returns_same_content_on_second_request);

    teardown();

    std::cout << "\n";
}