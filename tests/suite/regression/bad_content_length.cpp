// Regression coverage: a non-numeric Content-Length header.
//
// Server.cpp does its own lightweight Content-Length scan (separate
// from HttpParser's own parsing) so it knows how much body to read
// before handing the raw bytes to HttpParser::parse(). A value
// std::from_chars can't fully consume - "abc", "12x", empty - must be
// rejected with 400 Bad Request rather than silently treated as 0 (the
// previous behavior, before this was fixed to check from_chars' result).

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

// Verifies a non-numeric Content-Length is rejected with 400, not
// silently treated as a zero-length body.
static void rejects_non_numeric_content_length() {
    const uint16_t port = 18513;

    Router router;
    router.post("/echo", [](const HttpRequest& request, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
        response.setBody(request.body());
    });

    Server server(router, /*threadCount=*/2);
    CHK(server.start(port));

    std::thread serverThread([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::string request = "POST /echo HTTP/1.1\r\n"
                          "Host: h\r\n"
                          "Content-Length: abc\r\n"
                          "\r\n";

    std::string response = sendRawRequest(port, request);
    CHK(response.find("400") != std::string::npos);

    server.stop();
    serverThread.join();
}

// Executes the malformed Content-Length regression case.
static void run_tests() {
    RUN(rejects_non_numeric_content_length);
}

REGISTER_TEST_SUITE();
