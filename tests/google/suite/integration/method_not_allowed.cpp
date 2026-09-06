// Regression coverage: runChain()'s MethodNotAllowed branch.
//
// When Router::dispatch() finds a route whose path matches but whose
// method doesn't, it reports DispatchResult::MethodNotAllowed -
// distinct from NotFound, and mapped to a different status code
// (405, not 404) in runChain(). A route table with only a GET handler
// registered, hit with POST, is the minimal case that reaches this
// branch instead of NotFound.

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

namespace {

#ifdef _WIN32
using SocketHandle = SOCKET;
constexpr SocketHandle kInvalidSocket = INVALID_SOCKET;

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
constexpr SocketHandle kInvalidSocket = -1;

void closeSocket(SocketHandle fd) {
    ::close(fd);
}
#endif

std::string sendRawRequest(uint16_t port, const std::string& raw) {
    SocketHandle fd = ::socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    ::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
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

// Verifies a method mismatch on an otherwise-matching path returns
// 405, not 404.
TEST(MethodNotAllowed, Reports405OnMethodMismatch) {
    const uint16_t port = 18516;

    Router router;
    router.get("/resource", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
        response.setBody("ok");
    });

    Server server(router, /*threadCount=*/2);
    ASSERT_TRUE(server.start(port));

    std::thread serverThread([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::string response =
        sendRawRequest(port, "POST /resource HTTP/1.1\r\nHost: h\r\nContent-Length: 0\r\n\r\n");
    EXPECT_NE(response.find("405"), std::string::npos);

    server.stop();
    serverThread.join();
}
