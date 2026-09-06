// Full-stack streaming disconnect-detection integration test.
//
// Coverage:
// - A client that closes its connection mid-stream causes the next
//   send() to fail and isConnected() to flip false, rather than the
//   handler looping forever writing into a dead socket
// - The server survives the disconnect (no crash, no hang) and can
//   still handle a normal request afterward - mirroring the existing
//   header_disconnect.cpp/body_disconnect.cpp regression pattern for
//   the streaming code path

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

#include <atomic>
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

} // namespace

// Verifies a mid-stream client disconnect is noticed by the handler
// (via SseConnection::isConnected() going false) rather than hanging
// or crashing, and that the server keeps serving normal requests
// afterward.
TEST(SseDisconnect, HandlerNoticesMidStreamDisconnect) {
    const uint16_t port = 18612;

    std::atomic<bool> disconnectNoticed{false};

    Router router;
    router.get("/health", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
        response.setBody("ok");
    });
    router.stream("/events", [&disconnectNoticed](const HttpRequest&, SseConnection& sse) {
        // Loop well past when the test client disconnects (see
        // below) - a working implementation should exit this loop
        // early rather than running all 50 iterations.
        for (int i = 0; i < 50; ++i) {
            if (!sse.send("tick", std::to_string(i))) {
                disconnectNoticed = true;
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    });

    Server server(router, /*threadCount=*/2);
    ASSERT_TRUE(server.start(port));

    std::thread serverThread([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    SocketHandle fd = connectToLoopback(port);
    std::string request = "GET /events HTTP/1.1\r\nHost: h\r\n\r\n";
    ::send(fd, request.data(), static_cast<int>(request.size()), 0);

    // Read the preamble plus at least one event so the handler is
    // definitely inside its send loop, then disconnect abruptly.
    char buffer[4096];
    (void)::recv(fd, buffer, sizeof(buffer), 0);
    closeSocket(fd);

    // Give the handler's next send() attempt time to fail and be
    // noticed (well under the 50 * 20ms it would take to run the
    // full loop to completion if disconnects went undetected).
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_TRUE(disconnectNoticed.load());

    // The server (and its thread pool) must still work normally.
    std::string response = sendRawRequest(port, "GET /health HTTP/1.1\r\nHost: h\r\n\r\n");
    EXPECT_NE(response.find("200"), std::string::npos);
    EXPECT_NE(response.find("ok"), std::string::npos);

    server.stop();
    serverThread.join();
}
