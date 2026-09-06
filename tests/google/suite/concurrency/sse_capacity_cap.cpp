// Concurrency test for ServerConfig::maxStreamingConnections /
// Server's thread-pool-capacity guard (Phase 4 of the SSE extension).
//
// Coverage:
// - With the cap resolved to 1 (threadCount=2, maxStreamingConnections
//   left at its "0 = auto" default), a first streaming connection
//   upgrades normally
// - A second, concurrent streaming connection while the first is
//   still open is rejected with 503 Service Unavailable rather than
//   queuing forever or being silently dropped
// - After the first connection ends, the slot frees up and a new
//   streaming connection can upgrade again

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
using namespace FalconHTTP::Config;
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

std::string recvChunk(SocketHandle fd) {
    char buffer[4096];
    int n = ::recv(fd, buffer, sizeof(buffer), 0);
    if (n <= 0) {
        return "";
    }
    return std::string(buffer, static_cast<std::size_t>(n));
}

std::string recvAll(SocketHandle fd) {
    std::string response;
    char buffer[4096];
    int n;
    while ((n = ::recv(fd, buffer, sizeof(buffer), 0)) > 0) {
        response.append(buffer, static_cast<std::size_t>(n));
    }
    return response;
}

} // namespace

// Verifies the auto-resolved streaming cap (half of threadCount,
// minimum 1) rejects a second concurrent subscriber with 503, then
// admits a new one once the first disconnects.
TEST(SseCapacityCap, ConcurrentStreamsBeyondCapGet503) {
    const uint16_t port = 18615;

    ServerConfig config;
    config.threadCount = 2; // auto cap resolves to 1
    config.port = port;

    Router router;
    router.stream("/events", [](const HttpRequest&, SseConnection& sse) {
        for (int i = 0; i < 20 && sse.isConnected(); ++i) {
            (void)sse.send("tick", std::to_string(i));
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });

    Server server(router, config);
    ASSERT_TRUE(server.start());

    std::thread serverThread([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::string request = "GET /events HTTP/1.1\r\nHost: h\r\n\r\n";

    SocketHandle first = connectToLoopback(port);
    ::send(first, request.data(), static_cast<int>(request.size()), 0);
    std::string firstPreamble = recvChunk(first);
    ASSERT_TRUE(firstPreamble.starts_with("HTTP/1.1 200 OK\r\n"));
    ASSERT_NE(firstPreamble.find("text/event-stream"), std::string::npos);

    SocketHandle second = connectToLoopback(port);
    ::send(second, request.data(), static_cast<int>(request.size()), 0);
    std::string secondResponse = recvAll(second);
    EXPECT_TRUE(secondResponse.starts_with("HTTP/1.1 503 Service Unavailable\r\n"));
    closeSocket(second);

    // Free the slot, then confirm a new connection can use it. The
    // server only notices the disconnect (and releases the slot) on
    // the handler's next scheduled send() attempt (its loop sleeps
    // 50ms between sends) - poll/retry rather than assume a single
    // fixed delay is always enough under scheduler load.
    closeSocket(first);

    std::string thirdPreamble;
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (std::chrono::steady_clock::now() < deadline) {
        SocketHandle attempt = connectToLoopback(port);
        ::send(attempt, request.data(), static_cast<int>(request.size()), 0);
        std::string response = recvChunk(attempt);

        if (response.starts_with("HTTP/1.1 200 OK\r\n")) {
            thirdPreamble = response;
            closeSocket(attempt);
            break;
        }

        closeSocket(attempt);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    EXPECT_TRUE(thirdPreamble.starts_with("HTTP/1.1 200 OK\r\n"));

    server.stop();
    serverThread.join();
}
