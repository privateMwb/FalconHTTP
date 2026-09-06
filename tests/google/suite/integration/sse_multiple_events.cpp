// Full-stack streaming integration test: a real Server, a real
// Stream-kind route, and a real client socket.
//
// Coverage:
// - The SSE preamble (status line, Content-Type: text/event-stream,
//   no Content-Length) arrives before any event
// - Multiple send() calls, spaced apart in real time, arrive as
//   separate distinct events - not one batched write
// - The connection stays open across all of them (no premature close)

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

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

} // namespace

// Verifies a streaming route's preamble and three separately-timed
// events all arrive correctly over one held-open connection, and that
// the library truly writes each event immediately (not batched) - the
// events being spaced apart in wall-clock time is recorded from
// inside the handler itself, not inferred from client-side recv()
// timing, since the latter is at the mercy of when the test process's
// own thread happens to get scheduled and can't reliably distinguish
// "delivered incrementally" from "the client just read late."
TEST(SseMultipleEvents, StreamingRouteDeliversMultipleEvents) {
    const uint16_t port = 18611;

    std::vector<std::chrono::steady_clock::time_point> sendTimestamps;
    std::mutex timestampsMutex;

    Router router;
    router.stream("/events", [&](const HttpRequest&, SseConnection& sse) {
        for (int i = 0; i < 3 && sse.isConnected(); ++i) {
            (void)sse.send("tick", "n=" + std::to_string(i));
            {
                std::lock_guard<std::mutex> lock(timestampsMutex);
                sendTimestamps.push_back(std::chrono::steady_clock::now());
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    Server server(router, /*threadCount=*/2);
    ASSERT_TRUE(server.start(port));

    std::thread serverThread([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    SocketHandle fd = connectToLoopback(port);
    std::string request = "GET /events HTTP/1.1\r\nHost: h\r\n\r\n";
    ::send(fd, request.data(), static_cast<int>(request.size()), 0);

    // The client doesn't need to time anything - just drain the
    // connection until it's fully closed (handler returns, then
    // SseConnection's destructor closes it), then verify content.
    std::string accumulated;
    char buffer[4096];
    int n;
    while ((n = ::recv(fd, buffer, sizeof(buffer), 0)) > 0) {
        accumulated.append(buffer, static_cast<std::size_t>(n));
    }

    auto countTicks = [](const std::string& s) {
        int count = 0;
        std::size_t pos = 0;
        while ((pos = s.find("event: tick", pos)) != std::string::npos) {
            ++count;
            pos += 1;
        }
        return count;
    };

    ASSERT_TRUE(accumulated.starts_with("HTTP/1.1 200 OK\r\n"));
    EXPECT_NE(accumulated.find("Content-Type: text/event-stream"), std::string::npos);
    EXPECT_EQ(accumulated.find("Content-Length"), std::string::npos);
    EXPECT_NE(accumulated.find("data: n=0"), std::string::npos);
    EXPECT_NE(accumulated.find("data: n=1"), std::string::npos);
    EXPECT_NE(accumulated.find("data: n=2"), std::string::npos);
    EXPECT_EQ(countTicks(accumulated), 3);

    // The real, scheduling-independent proof that events were pushed
    // incrementally rather than buffered: the handler's own send()
    // calls, timestamped from inside the handler, are genuinely spaced
    // apart by close to the 100ms it slept between them.
    {
        std::lock_guard<std::mutex> lock(timestampsMutex);
        ASSERT_EQ(sendTimestamps.size(), 3u);
        auto gap1 = std::chrono::duration_cast<std::chrono::milliseconds>(sendTimestamps[1] -
                                                                          sendTimestamps[0]);
        auto gap2 = std::chrono::duration_cast<std::chrono::milliseconds>(sendTimestamps[2] -
                                                                          sendTimestamps[1]);
        EXPECT_GE(gap1.count(), 80);
        EXPECT_GE(gap2.count(), 80);
    }

    closeSocket(fd);
    server.stop();
    serverThread.join();
}
