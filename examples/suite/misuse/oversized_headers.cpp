// Sending a request with too large a header block.
//
// Demonstrates:
// - maxHeaderSize is enforced against the request line + header bytes
//   read so far, before the terminating blank line is even found
// - What actually happens when it's exceeded: 431 Request Header Fields
//   Too Large (this used to incorrectly return 400 - see Server.cpp)
// - The connection is still closed cleanly afterward, not dropped
//
// Note: this example opens a real TCP connection to a FalconHTTP server
// running in the same process, using plain POSIX sockets to act as the
// client (FalconHTTP's own Socket class has no connect() - it's
// listener/server-side only). POSIX-only for simplicity.

#include <support/framework.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace FalconHTTP;

namespace {
// A minimal blocking client: connects, sends raw bytes, reads
// whatever comes back until the peer closes.
std::string sendRawRequest(uint16_t port, const std::string& raw) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return "";

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        ::close(fd);
        return "";
    }

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

static void run_examples() {

    setTitle("A Server With a Small Header Cap");

    Config::ServerConfig config;
    config.port = 18081;
    config.maxHeaderSize = 256; // deliberately tiny, for this example only

    Routing::Router router;
    router.get("/", [](const HTTP::HttpRequest&, HTTP::HttpResponse& response) {
        response.setStatus(HTTP::HttpStatus::Ok);
        response.setBody("ok");
    });

    Core::Server server(router, config);

    if (!server.start()) {
        std::cout << "port 18081 unavailable, skipping the rest of this example\n";
        return;
    }

    std::thread serverThread([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // let run() reach accept()

    // A single oversized header pushes the request well past 256 bytes
    // before the terminating blank line is ever reached.
    setTitle("Sending an Oversized Header Block");

    std::string oversizedHeader(1024, 'x');
    std::string request = "GET / HTTP/1.1\r\n"
                          "Host: localhost\r\n"
                          "X-Padding: " +
                          oversizedHeader +
                          "\r\n"
                          "\r\n";

    std::string response = sendRawRequest(config.port, request);

    bool got431 = response.find("431") != std::string::npos;
    std::cout << "response starts with 431 : " << got431 << "\n";
    std::cout << "first line                : " << response.substr(0, response.find("\r\n"))
              << "\n";

    server.stop();
    serverThread.join();
}

REGISTER_EXAMPLE_SUITE();
