// Regression coverage: handleConnection()'s top-level catch block.
//
// A request that passes Server's own pre-checks (a "\r\n\r\n" is
// found, so the header-search loop never runs; no Content-Length, so
// the body loop never runs) can still fail inside
// HTTP::HttpParser::parse() itself - an empty request line has no
// method/path/version to split on, and parse() throws. That exception
// must be caught and turned into 500 Internal Server Error rather than
// propagating out of handleConnection() and taking down the pool
// thread (and, per Socket::send()'s SIGPIPE behavior, potentially the
// whole process).

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

// Verifies a request line HttpParser::parse() can't split (empty,
// here) results in 500, not a crash or a hang - and that the server
// keeps serving normally afterward.
static void reports_500_on_parse_failure() {
    const uint16_t port = 18515;

    Router router;
    router.get("/health", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
        response.setBody("ok");
    });

    Server server(router, /*threadCount=*/2);
    CHK(server.start(port));

    std::thread serverThread([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // The blank line alone: Server finds "\r\n\r\n" immediately (an
    // empty header block), so its own pre-checks all pass - but the
    // "request line" HttpParser::parse() extracts is empty, which it
    // cannot split into method/path/version.
    std::string response = sendRawRequest(port, "\r\n\r\n");
    CHK(response.find("500") != std::string::npos);

    std::string followUp = sendRawRequest(port, "GET /health HTTP/1.1\r\nHost: h\r\n\r\n");
    CHK(followUp.find("200") != std::string::npos);

    server.stop();
    serverThread.join();
}

// Executes the parse-failure regression case.
static void run_tests() {
    RUN(reports_500_on_parse_failure);
}

REGISTER_TEST_SUITE();
