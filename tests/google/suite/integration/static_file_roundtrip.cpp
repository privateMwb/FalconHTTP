// Static file serving roundtrip integration test suite:
// StaticFileServer -> FileCache -> HttpResponse.
//
// Coverage:
// - A file on disk is found, served with 200, correct Content-Type,
//   and correct body content
// - A second request for the same path is served from cache and
//   returns identical content
// - A request for a nonexistent file returns false (caller sets 404)
// - A path-traversal attempt ("..") is rejected rather than escaping
//   the configured root directory
//
// Each test creates and cleans up its own temporary directory under
// std::filesystem::temp_directory_path() so tests don't interfere
// with each other or leave files behind.

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

// clang-format off
#include <filesystem> // std::filesystem
#include <fstream>    // std::ofstream
// clang-format on

using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::FileServer;

namespace {

// Creates a fresh, empty temporary directory and returns its path.
std::filesystem::path makeTempDir(const char* suffix) {
    std::filesystem::path dir =
        std::filesystem::temp_directory_path() / (std::string("falconhttp_static_test_") + suffix);
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    return dir;
}

} // namespace

// Verifies a file present under the root directory is found, served
// with 200-equivalent success, the correct Content-Type, and its
// exact content as the body.
TEST(StaticFileRoundtrip, ServesExistingFileWithCorrectContent) {
    std::filesystem::path dir = makeTempDir("existing");
    {
        std::ofstream file(dir / "index.html");
        file << "<h1>Hello</h1>";
    }

    StaticFileServer server(dir.string(), /*cacheCapacity=*/16);
    HttpResponse response;

    ASSERT_TRUE(server.serve("/index.html", response));
    EXPECT_EQ(response.body(), "<h1>Hello</h1>");
    EXPECT_EQ(response.header("content-type"), "text/html");

    std::filesystem::remove_all(dir);
}

// Verifies a second request for the same path returns identical
// content, whether or not it was actually served from cache
// internally (FileCache's own hit/miss behavior is covered directly
// in the unit suite - this checks the observable contract).
TEST(StaticFileRoundtrip, RepeatedRequestReturnsSameContent) {
    std::filesystem::path dir = makeTempDir("repeated");
    {
        std::ofstream file(dir / "data.json");
        file << "{\"ok\":true}";
    }

    StaticFileServer server(dir.string(), /*cacheCapacity=*/16);

    HttpResponse firstResponse;
    ASSERT_TRUE(server.serve("/data.json", firstResponse));

    HttpResponse secondResponse;
    ASSERT_TRUE(server.serve("/data.json", secondResponse));

    EXPECT_EQ(firstResponse.body(), secondResponse.body());
    EXPECT_EQ(secondResponse.body(), "{\"ok\":true}");

    std::filesystem::remove_all(dir);
}

// Verifies a request for a file that doesn't exist under the root
// returns false rather than throwing or serving something else.
TEST(StaticFileRoundtrip, ReturnsFalseForMissingFile) {
    std::filesystem::path dir = makeTempDir("missing");

    StaticFileServer server(dir.string(), /*cacheCapacity=*/16);
    HttpResponse response;

    EXPECT_FALSE(server.serve("/does-not-exist.html", response));

    std::filesystem::remove_all(dir);
}

// Verifies a path-traversal attempt does not escape the configured
// root directory, even if a file happens to exist at the resolved
// location outside it.
TEST(StaticFileRoundtrip, RejectsPathTraversalAttempt) {
    std::filesystem::path dir = makeTempDir("traversal");
    std::filesystem::create_directories(dir / "public");
    {
        std::ofstream file(dir / "secret.txt");
        file << "should not be reachable";
    }

    StaticFileServer server((dir / "public").string(), /*cacheCapacity=*/16);
    HttpResponse response;

    EXPECT_FALSE(server.serve("/../secret.txt", response));

    std::filesystem::remove_all(dir);
}
