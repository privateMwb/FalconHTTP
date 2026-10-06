// Full-stack HttpClient integration test suite: a real Server, a real
// Router, and HttpClient talking to it over loopback.
//
// Coverage:
// - GET returns status, headers, and body intact
// - POST delivers its body and a custom header to the handler
// - An error status (404) is a returned response, not a failure
// - A 307 redirect is returned as-is (not followed) with its Location
//   header, proving Server emits 307 and the client parses it
// - A refused connection yields nullopt
// - A handler slower than ioTimeout yields nullopt, not a hang

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <thread>

using namespace FalconHTTP::Client;
using namespace FalconHTTP::Core;
using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Routing;

namespace {

// Runs a Server on a background thread for the life of the scope.
class ServerFixture {
  public:
    ServerFixture(Router& router, uint16_t port) : server_(router, /*threadCount=*/2) {
        started_ = server_.start(port);
        thread_ = std::thread([this]() { server_.run(); });
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    ~ServerFixture() {
        server_.stop();
        if (thread_.joinable())
            thread_.join();
    }

    [[nodiscard]] bool started() const noexcept {
        return started_;
    }

  private:
    Server server_;
    std::thread thread_;
    bool started_ = false;
};

} // namespace

// Verifies GET returns the handler's status, a header, and the body.
TEST(ClientRoundtrip, GetReturnsStatusAndBody) {
    const uint16_t port = 18711;
    Router router;
    router.get("/hello", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
        response.setHeader("X-Node", "A");
        response.setBody("hi there");
    });
    ServerFixture fixture(router, port);
    ASSERT_TRUE(fixture.started());

    HttpClient client;
    auto response = client.get("127.0.0.1", port, "/hello");

    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->statusCode, 200);
    EXPECT_TRUE(response->ok());
    EXPECT_EQ(response->body, "hi there");
    EXPECT_EQ(response->headers["x-node"], "A");
    EXPECT_EQ(response->headers["content-length"], "8");
}

// Verifies a POST's body and a caller-supplied header reach the handler.
TEST(ClientRoundtrip, PostDeliversBodyAndHeader) {
    const uint16_t port = 18712;
    Router router;
    router.post("/echo", [](const HttpRequest& request, HttpResponse& response) {
        response.setStatus(HttpStatus::Created);
        response.setBody(request.body() + "|" + request.header("x-trace"));
    });
    ServerFixture fixture(router, port);
    ASSERT_TRUE(fixture.started());

    HashMapPro::HashMap<std::string, std::string> headers;
    headers["X-Trace"] = "t-42";
    headers["Content-Length"] = "999"; // must be ignored: the client owns it.

    HttpClient client;
    auto response = client.post("127.0.0.1", port, "/echo", "{\"k\":1}", headers);

    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->statusCode, 201);
    EXPECT_EQ(response->body, "{\"k\":1}|t-42");
}

// Verifies a 404 comes back as a normal response with ok() == false.
TEST(ClientRoundtrip, ErrorStatusIsAResponse) {
    const uint16_t port = 18713;
    Router router;
    router.get("/known", [](const HttpRequest&, HttpResponse&) {});
    ServerFixture fixture(router, port);
    ASSERT_TRUE(fixture.started());

    HttpClient client;
    auto response = client.get("127.0.0.1", port, "/missing");

    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->statusCode, 404);
    EXPECT_FALSE(response->ok());
}

// Verifies a 307 is returned unfollowed, Location intact, and that a
// PUT's method survives into the redirect decision (the handler sees PUT).
TEST(ClientRoundtrip, Redirect307ReturnedUnfollowed) {
    const uint16_t port = 18714;
    Router router;
    router.put("/kv/a", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::TemporaryRedirect);
        response.setHeader("Location", "http://127.0.0.1:9999/kv/a");
    });
    ServerFixture fixture(router, port);
    ASSERT_TRUE(fixture.started());

    HttpClient client;
    auto response = client.put("127.0.0.1", port, "/kv/a", "value");

    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->statusCode, 307);
    EXPECT_EQ(response->headers["location"], "http://127.0.0.1:9999/kv/a");
    EXPECT_FALSE(response->ok());
}

// Verifies connecting to a port nobody listens on yields nullopt.
TEST(ClientRoundtrip, RefusedConnectionIsNullopt) {
    HttpClient client;
    auto response = client.get("127.0.0.1", 18719, "/anything");

    EXPECT_FALSE(response.has_value());
}

// Verifies a handler slower than ioTimeout yields nullopt promptly.
TEST(ClientRoundtrip, SlowHandlerTimesOut) {
    const uint16_t port = 18715;
    Router router;
    router.get("/slow", [](const HttpRequest&, HttpResponse& response) {
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
        response.setStatus(HttpStatus::Ok);
        response.setBody("too late");
    });
    ServerFixture fixture(router, port);
    ASSERT_TRUE(fixture.started());

    ClientOptions options;
    options.ioTimeout = std::chrono::milliseconds(100);
    HttpClient client(options);

    const auto begin = std::chrono::steady_clock::now();
    auto response = client.get("127.0.0.1", port, "/slow");
    const auto elapsed = std::chrono::steady_clock::now() - begin;

    EXPECT_FALSE(response.has_value());
    EXPECT_TRUE(elapsed < std::chrono::milliseconds(500)); // gave up well before the handler finished.
}
