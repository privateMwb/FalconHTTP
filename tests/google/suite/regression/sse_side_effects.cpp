// Regression coverage for the SSE extension's two riskiest
// side-effects on pre-existing behavior.
//
// Coverage:
// - A normal (Normal-kind) route's response is completely unaffected
//   by a Router also having Stream-kind routes registered, and by a
//   streaming connection having just been used on the same server -
//   still gets Content-Length, still gets Connection: close, still
//   gets its exact body (Server::handleConnection()'s normal
//   one-shot path, byte for byte)
// - Cors middleware's headers, set on the shared HttpResponse before
//   Server::runChain() detects a Stream-kind match, actually reach
//   the SSE preamble - answering the roadmap's open "CORS on
//   streaming responses" question in the affirmative rather than
//   leaving it an assumption

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

using namespace FalconHTTP::Core;
using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Routing;
using namespace FalconHTTP::Middleware;
using namespace FalconHTTP::Streaming;

namespace {

#ifdef _WIN32
using SocketHandle = SOCKET;

struct WinsockGuard {
    WinsockGuard() {
        WSADATA data;
        WSAStartup(MAKEWORD(2, 2), &data);
    }
    ~WinsockGuard() {
        WSACleanup();
    }
};
const WinsockGuard winsockGuard;

void closeSocket(SocketHandle fd) {
    ::closesocket(fd);
}
#else
using SocketHandle = int;

void closeSocket(SocketHandle fd) {
    ::close(fd);
}
#endif

SocketHandle connectToLoopback(uint16_t port) {
    SocketHandle fd = ::socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    ::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    return fd;
}

std::string sendRawRequest(uint16_t port, const std::string& raw) {
    SocketHandle fd = connectToLoopback(port);
    ::send(fd, raw.data(), static_cast<int>(raw.size()), 0);

    std::string response;
    char buffer[4096];
    int n;
    while ((n = ::recv(fd, buffer, sizeof(buffer), 0)) > 0) {
        response.append(buffer, static_cast<std::size_t>(n));
    }

    closeSocket(fd);
    return response;
}

std::string recvChunk(SocketHandle fd) {
    char buffer[4096];
    int n = ::recv(fd, buffer, sizeof(buffer), 0);
    if (n <= 0) {
        return "";
    }
    return std::string(buffer, static_cast<std::size_t>(n));
}

} // namespace

// Verifies a normal route's response is exactly as it was before
// streaming existed, even on a Router that also has a Stream-kind
// route, and even right after that streaming route was used.
TEST(SseSideEffects, NormalRouteUnaffectedByStreamingRoute) {
    const uint16_t port = 18613;

    Router router;
    router.get("/ping", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
        response.setBody("pong");
    });
    router.stream("/events",
                  [](const HttpRequest&, SseConnection& sse) { (void)sse.send("tick", "0"); });

    Server server(router, /*threadCount=*/2);
    ASSERT_TRUE(server.start(port));

    std::thread serverThread([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Use the streaming route first, let it finish and release its
    // connection, then confirm the normal route is untouched.
    SocketHandle sseFd = connectToLoopback(port);
    std::string sseRequest = "GET /events HTTP/1.1\r\nHost: h\r\n\r\n";
    ::send(sseFd, sseRequest.data(), static_cast<int>(sseRequest.size()), 0);
    (void)recvChunk(sseFd);
    closeSocket(sseFd);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::string response = sendRawRequest(port, "GET /ping HTTP/1.1\r\nHost: h\r\n\r\n");

    EXPECT_TRUE(response.starts_with("HTTP/1.1 200 OK\r\n"));
    EXPECT_NE(response.find("Content-Length: 4"), std::string::npos);
    EXPECT_NE(response.find("Connection: close"), std::string::npos);
    EXPECT_TRUE(response.ends_with("pong"));

    server.stop();
    serverThread.join();
}

// Verifies Cors middleware's headers, set on the shared HttpResponse
// ahead of Server::runChain()'s streaming branch, are forwarded into
// the SSE preamble rather than silently dropped.
TEST(SseSideEffects, CorsHeadersReachSsePreamble) {
    const uint16_t port = 18614;

    Router router;
    router.stream("/events",
                  [](const HttpRequest&, SseConnection& sse) { (void)sse.send("tick", "0"); });

    Server server(router, /*threadCount=*/2);
    server.use(Cors());
    ASSERT_TRUE(server.start(port));

    std::thread serverThread([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    SocketHandle fd = connectToLoopback(port);
    std::string request = "GET /events HTTP/1.1\r\nHost: h\r\nOrigin: http://example.com\r\n\r\n";
    ::send(fd, request.data(), static_cast<int>(request.size()), 0);

    std::string preamble = recvChunk(fd);
    EXPECT_NE(preamble.find("Content-Type: text/event-stream"), std::string::npos);
    EXPECT_NE(preamble.find("access-control-allow-origin"), std::string::npos);

    closeSocket(fd);
    server.stop();
    serverThread.join();
}
