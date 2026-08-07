// Regression test: every response must include "Connection: close",
// since FalconHTTP does not implement keep-alive and closes every
// connection after exactly one response.
//
// The original bug: HttpSerializer never emitted Connection: close,
// but Server::handleConnection() always closes the socket after
// sending the response regardless. HTTP/1.1 clients default to
// assuming keep-alive unless told otherwise - without this header, a
// spec-compliant client could wait on or attempt to reuse a socket
// the server had already torn down.
//
// SCOPE: this specifically needs a real Server actually closing a
// real socket after responding, which only happens inside private
// Server::handleConnection() reached via Listener::accept(). Since
// Socket has no client-side connect() anywhere in the public API,
// this test dials in with minimal raw POSIX socket code instead of
// going through FalconHTTP's own types - the only way to reach this
// code path at all currently. See the file-level notes in
// oversized_body_rejected.cpp and oversized_header_rejected.cpp for
// the same rationale; all three share this approach.
//
// POSIX only (uses <sys/socket.h> directly) - matches the project's
// Termux/Android development environment. Uses a fixed port rather
// than an ephemeral one because Listener::port() only reports the
// literal value passed to start(), not the OS-assigned port for
// start(0) - so an ephemeral port can't currently be discovered by a
// caller. If this port is in use on the machine running the suite,
// this test will fail to start the server; pick a different constant
// below if that happens.

#include <support/framework.h>

// clang-format off
#include <thread>  // std::thread
#include <chrono>  // std::chrono::milliseconds

#include <sys/socket.h> // socket, connect, send, recv
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h>  // inet_pton, htons
#include <unistd.h>     // close
// clang-format on

using namespace FalconHTTP::Core;
using namespace FalconHTTP::Routing;
using namespace FalconHTTP::HTTP;

namespace {

constexpr uint16_t TestPort = 18473;

// Connects to 127.0.0.1:port as a raw TCP client, sends `request`,
// then reads until the server closes the connection (EOF) and
// returns everything read. Returns an empty string on any
// connection failure.
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

// Verifies a real response from a live Server includes
// "Connection: close", and that the server actually does close the
// socket afterward (the raw client's read loop terminates on EOF
// rather than hanging or timing out).
static void live_response_includes_connection_close() {
    Router router;
    router.get("/health", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
        response.setBody("ok");
    });

    Server server(router, /*threadCount=*/2);
    CHK(server.start(TestPort));

    std::thread runner([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::string response = sendRawRequest(TestPort, "GET /health HTTP/1.1\r\nHost: h\r\n\r\n");

    server.stop();
    runner.join();

    CHK(response.find("Connection: close\r\n") != std::string::npos);
    CHK(response.starts_with("HTTP/1.1 200 OK\r\n"));
    CHK(response.ends_with("ok"));
}

// Executes all Connection: close regression test cases.
static void run_tests() {
    RUN(live_response_includes_connection_close);
}

REGISTER_TEST_SUITE();
