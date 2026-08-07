// Regression test: a request whose header block exceeds
// ServerConfig::maxHeaderSize before the terminating CRLFCRLF is
// found must be rejected with 431 Request Header Fields Too Large.
//
// The original issue: same DoS class as oversized_body_rejected.cpp,
// but on the header side - Server had no cap on how much it would
// accumulate while still searching for the blank line terminating the
// headers, so a client that never sends one could grow the buffer
// unboundedly. Fixed by checking the accumulated buffer size against
// maxHeaderSize_ on each iteration of that wait loop.
//
// SCOPE: see connection_close_sent.cpp for why this needs a live
// Server and raw POSIX client code, and the fixed-port caveat (this
// file uses a different port to avoid colliding with the other two
// live-server regression tests if all three run in the same process).

#include <support/framework.h>

// clang-format off
#include <thread>  // std::thread
#include <chrono>  // std::chrono::milliseconds
#include <string>  // std::string

#include <sys/socket.h> // socket, connect, send, recv
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h>  // inet_pton, htons
#include <unistd.h>     // close
// clang-format on

using namespace FalconHTTP::Core;
using namespace FalconHTTP::Routing;
using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Config;

namespace {

constexpr uint16_t TestPort = 18475;

// See connection_close_sent.cpp for details on this helper.
std::string sendRawRequest(uint16_t port, const std::string& request) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return {};

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    ::inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        ::close(fd);
        return {};
    }

    ::send(fd, request.data(), request.size(), 0);

    std::string response;
    char buffer[4096];
    ssize_t n;
    while ((n = ::recv(fd, buffer, sizeof(buffer), 0)) > 0) {
        response.append(buffer, static_cast<std::size_t>(n));
    }

    ::close(fd);
    return response;
}

} // namespace

// Verifies a header block larger than the configured maxHeaderSize,
// sent without ever completing the terminating blank line, is
// rejected with 431.
static void oversized_unterminated_headers_are_rejected_with_431() {
    ServerConfig config;
    config.maxHeaderSize = 64; // tiny cap to keep the test fast

    Router router;
    router.get("/x", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
    });

    Server server(router, config);
    CHK(server.start(TestPort));

    std::thread runner([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Well over 64 bytes, and deliberately never terminated with the
    // blank line that would end the header block.
    std::string request = "GET /x HTTP/1.1\r\nX-Pad: " + std::string(200, 'a') + "\r\n";

    std::string response = sendRawRequest(TestPort, request);

    server.stop();
    runner.join();

    CHK(response.starts_with("HTTP/1.1 431"));
}

// Verifies a normal, small request under the configured maxHeaderSize
// is accepted normally - the cap must not be overly aggressive.
static void small_header_block_is_accepted() {
    ServerConfig config;
    config.maxHeaderSize = 64;

    Router router;
    router.get("/x", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
        response.setBody("fine");
    });

    Server server(router, config);
    CHK(server.start(TestPort));

    std::thread runner([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::string response = sendRawRequest(TestPort, "GET /x HTTP/1.1\r\nHost: h\r\n\r\n");

    server.stop();
    runner.join();

    CHK(response.starts_with("HTTP/1.1 200 OK\r\n"));
    CHK(response.ends_with("fine"));
}

// Executes all oversized-header regression test cases.
static void run_tests() {
    RUN(oversized_unterminated_headers_are_rejected_with_431);
    RUN(small_header_block_is_accepted);
}

REGISTER_TEST_SUITE();
