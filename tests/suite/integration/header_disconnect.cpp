// Regression coverage: a client that disconnects before the header
// block is complete.
//
// While handleConnection() is still looking for the terminating
// "\r\n\r\n", a receiveAvailable() call returning empty (peer closed
// mid-header) must close the connection cleanly rather than looping
// forever waiting for headers that will never arrive.

#include <support/framework.h>

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

// Sends a partial header block (no terminating blank line) and closes
// before the request is ever complete.
void sendPartialHeadersAndClose(uint16_t port) {
    SocketHandle fd = ::socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    ::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

    std::string partial = "GET /health HTTP/1.1\r\nHost: h\r\n"; // no closing \r\n
    ::send(fd, partial.data(), static_cast<int>(partial.size()), 0);

    closeSocket(fd);
}

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

// Verifies a mid-header disconnect doesn't hang the server, and that
// it can still handle a normal request afterward.
static void survives_disconnect_mid_headers() {
    const uint16_t port = 18512;

    Router router;
    router.get("/health", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
        response.setBody("ok");
    });

    Server server(router, /*threadCount=*/2);
    CHK(server.start(port));

    std::thread serverThread([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    sendPartialHeadersAndClose(port);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::string response = sendRawRequest(port, "GET /health HTTP/1.1\r\nHost: h\r\n\r\n");
    CHK(response.find("200") != std::string::npos);

    server.stop();
    serverThread.join();
}

// Executes the mid-header-disconnect regression case.
static void run_tests() {
    RUN(survives_disconnect_mid_headers);
}

REGISTER_TEST_SUITE();
