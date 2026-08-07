// Sending a request body larger than the configured cap.
//
// Demonstrates:
// - maxBodySize is checked against the declared Content-Length, before
//   Server reads the body itself
// - The rejection: 413 Payload Too Large (this used to incorrectly
//   return 400 - see Server.cpp), sent without buffering the oversized body
// - A Content-Length within the cap being accepted normally, for contrast
//
// Note: POSIX-only raw client, same rationale as oversized_headers.cpp -
// FalconHTTP's Socket class has no client-side connect().

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

    setTitle("A Server With a Small Body Cap");

    Config::ServerConfig config;
    config.port = 18082;
    config.maxBodySize = 16; // deliberately tiny, for this example only

    Routing::Router router;
    router.post("/echo", [](const HTTP::HttpRequest& request, HTTP::HttpResponse& response) {
        response.setStatus(HTTP::HttpStatus::Ok);
        response.setBody(request.body());
    });

    Core::Server server(router, config);

    if (!server.start()) {
        std::cout << "port 18082 unavailable, skipping the rest of this example\n";
        return;
    }

    std::thread serverThread([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Content-Length alone is enough to trigger the rejection - Server
    // checks it before reading any of the declared body.
    setTitle("Declaring a Body Larger Than the Cap");

    std::string oversizedBody(64, 'a');
    std::string oversizedRequest = "POST /echo HTTP/1.1\r\n"
                                   "Host: localhost\r\n"
                                   "Content-Length: " +
                                   std::to_string(oversizedBody.size()) +
                                   "\r\n"
                                   "\r\n" +
                                   oversizedBody;

    std::string oversizedResponse = sendRawRequest(config.port, oversizedRequest);

    bool got413 = oversizedResponse.find("413") != std::string::npos;
    std::cout << "response starts with 413 : " << got413 << "\n\n";

    // A body within the cap goes through normally.
    setTitle("A Body Within the Cap");

    std::string smallBody = "hello";
    std::string smallRequest = "POST /echo HTTP/1.1\r\n"
                               "Host: localhost\r\n"
                               "Content-Length: " +
                               std::to_string(smallBody.size()) +
                               "\r\n"
                               "\r\n" +
                               smallBody;

    std::string smallResponse = sendRawRequest(config.port, smallRequest);

    bool got200 = smallResponse.find("200") != std::string::npos;
    std::cout << "response starts with 200 : " << got200 << "\n";

    server.stop();
    serverThread.join();
}

REGISTER_EXAMPLE_SUITE();
