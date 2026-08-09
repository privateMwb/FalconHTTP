// Regression coverage: a connection that sends no data at all.
//
// handleConnection()'s very first receiveAvailable() call can return
// empty (peer connected and closed without writing anything) - that
// path just closes the connection with no response, distinct from
// every other branch in this function which requires at least a
// partial header block to reach. Confirms the server survives it and
// keeps serving subsequent connections normally.

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

// Connects, sends nothing, and closes immediately.
void connectAndCloseImmediately(uint16_t port) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    ::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
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

// Verifies an empty connection doesn't crash or hang the server, and
// that it can still handle a normal request afterward.
static void survives_connection_with_no_data() {
    const uint16_t port = 18511;

    Router router;
    router.get("/health", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
        response.setBody("ok");
    });

    Server server(router, /*threadCount=*/2);
    CHK(server.start(port));

    std::thread serverThread([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    connectAndCloseImmediately(port);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::string response = sendRawRequest(port, "GET /health HTTP/1.1\r\nHost: h\r\n\r\n");
    CHK(response.find("200") != std::string::npos);

    server.stop();
    serverThread.join();
}

// Executes the empty-connection regression case.
static void run_tests() {
    RUN(survives_connection_with_no_data);
}

REGISTER_TEST_SUITE();
