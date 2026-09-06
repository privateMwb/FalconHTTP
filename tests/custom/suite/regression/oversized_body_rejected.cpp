// Regression test: a request whose Content-Length exceeds
// ServerConfig::maxBodySize must be rejected with 413 Payload Too
// Large, without the server attempting to read/buffer the full body.
//
// The original issue: Server had no cap on Content-Length or the
// accumulated read buffer at all - a client could declare an
// arbitrary Content-Length and the server would keep growing its
// buffer waiting for that many bytes to arrive, an unauthenticated
// memory-exhaustion DoS. Fixed by checking Content-Length against
// maxBodySize_ immediately after headers are parsed, before entering
// the body-read loop, and responding 413 without ever trying to read
// the (possibly enormous, possibly never-arriving) body.
//
// SCOPE: see connection_close_sent.cpp for why this needs a live
// Server and raw POSIX client code, and the fixed-port caveat (this
// file uses a different port to avoid colliding with that test if
// both suites run in the same process).

#include <support/framework.h>

// clang-format off
#include <thread>  // std::thread
#include <chrono>  // std::chrono::milliseconds
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
// clang-format on

using namespace FalconHTTP::Core;
using namespace FalconHTTP::Routing;
using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Config;

namespace {

constexpr uint16_t TestPort = 18474;

#ifdef _WIN32
using SocketHandle = SOCKET;
using SocketLength = int;

inline void closeSocket(SocketHandle s) {
    ::closesocket(s);
}

constexpr SocketHandle InvalidSocket = INVALID_SOCKET;

#else

using SocketHandle = int;
using SocketLength = ssize_t;

inline void closeSocket(SocketHandle s) {
    ::close(s);
}

constexpr SocketHandle InvalidSocket = -1;

#endif

// See connection_close_sent.cpp for details on this helper.
std::string sendRawRequest(uint16_t port, const std::string& request) {
    SocketHandle fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == InvalidSocket)
        return {};

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (::inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
        closeSocket(fd);
        return {};
    }

    if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        closeSocket(fd);
        return {};
    }

#ifdef _WIN32
    ::send(fd, request.data(), static_cast<int>(request.size()), 0);
#else
    ::send(fd, request.data(), request.size(), 0);
#endif

    std::string response;
    char buffer[4096];

    SocketLength n;
    while ((n = ::recv(fd, buffer, sizeof(buffer), 0)) > 0) {
        response.append(buffer, static_cast<std::size_t>(n));
    }

    closeSocket(fd);
    return response;
}

} // namespace

// Verifies a request declaring a Content-Length above the configured
// maxBodySize is rejected with 413, without the client needing to
// actually send that many body bytes - the check must happen against
// the declared length before the read loop begins.
static void oversized_content_length_is_rejected_with_413() {
    ServerConfig config;
    config.maxBodySize = 16; // tiny cap to keep the test fast

    Router router;
    router.post("/upload", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
    });

    Server server(router, config);
    CHK(server.start(TestPort));

    std::thread runner([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    std::string request = "POST /upload HTTP/1.1\r\n"
                          "Host: h\r\n"
                          "Content-Length: 1000\r\n"
                          "\r\n";

    std::string response = sendRawRequest(TestPort, request);

    server.stop();
    runner.join();

    CHK(response.starts_with("HTTP/1.1 413"));
}

// Verifies a request within the configured maxBodySize is accepted
// normally - the cap must not be overly aggressive.
static void body_within_limit_is_accepted() {
    ServerConfig config;
    config.maxBodySize = 16;

    Router router;
    router.post("/upload", [](const HttpRequest& request, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
        response.setBody(request.body());
    });

    Server server(router, config);
    CHK(server.start(TestPort));

    std::thread runner([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    std::string request = "POST /upload HTTP/1.1\r\n"
                          "Host: h\r\n"
                          "Content-Length: 5\r\n"
                          "\r\n"
                          "hello";

    std::string response = sendRawRequest(TestPort, request);

    server.stop();
    runner.join();

    CHK(response.starts_with("HTTP/1.1 200 OK\r\n"));
    CHK(response.ends_with("hello"));
}

// Executes all oversized-body regression test cases.
static void run_tests() {
    RUN(oversized_content_length_is_rejected_with_413);
    RUN(body_within_limit_is_accepted);
}

REGISTER_TEST_SUITE();