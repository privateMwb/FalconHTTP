// Regression coverage: a client that disconnects before sending the
// full declared body.
//
// Once Content-Length is known and within maxBodySize_,
// handleConnection() loops on receiveAvailable() until it has that
// many body bytes. A receiveAvailable() call returning empty partway
// through (peer closed early) must close the connection rather than
// looping forever waiting for bytes that will never arrive.

#include <support/framework.h>

#include <chrono>
#include <string>
#include <thread>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace FalconHTTP::Core;
using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Routing;

namespace {

// Declares a 100-byte body via Content-Length, sends only 10, then
// closes before the rest ever arrives.
void sendPartialBodyAndClose(uint16_t port) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    ::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

    std::string request = "POST /echo HTTP/1.1\r\n"
                          "Host: h\r\n"
                          "Content-Length: 100\r\n"
                          "\r\n"
                          "1234567890"; // only 10 of the declared 100 bytes

    ::send(fd, request.data(), request.size(), 0);
    ::close(fd);
}

std::string sendRawRequest(uint16_t port, const std::string& raw) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    ::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    ::send(fd, raw.data(), raw.size(), 0);

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

// Verifies a mid-body disconnect doesn't hang the server, and that it
// can still handle a normal request afterward.
static void survives_disconnect_mid_body() {
    const uint16_t port = 18514;

    Router router;
    router.post("/echo", [](const HttpRequest& request, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
        response.setBody(request.body());
    });
    router.get("/health", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
        response.setBody("ok");
    });

    Server server(router, /*threadCount=*/2);
    CHK(server.start(port));

    std::thread serverThread([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    sendPartialBodyAndClose(port);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::string response = sendRawRequest(port, "GET /health HTTP/1.1\r\nHost: h\r\n\r\n");
    CHK(response.find("200") != std::string::npos);

    server.stop();
    serverThread.join();
}

// Executes the mid-body-disconnect regression case.
static void run_tests() {
    RUN(survives_disconnect_mid_body);
}

REGISTER_TEST_SUITE();
