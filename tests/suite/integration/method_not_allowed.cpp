// Regression coverage: runChain()'s MethodNotAllowed branch.
//
// When Router::dispatch() finds a route whose path matches but whose
// method doesn't, it reports DispatchResult::MethodNotAllowed -
// distinct from NotFound, and mapped to a different status code
// (405, not 404) in runChain(). A route table with only a GET handler
// registered, hit with POST, is the minimal case that reaches this
// branch instead of NotFound.

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

// Verifies a method mismatch on an otherwise-matching path returns
// 405, not 404.
static void reports_405_on_method_mismatch() {
    const uint16_t port = 18516;

    Router router;
    router.get("/resource", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
        response.setBody("ok");
    });

    Server server(router, /*threadCount=*/2);
    CHK(server.start(port));

    std::thread serverThread([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::string response =
        sendRawRequest(port, "POST /resource HTTP/1.1\r\nHost: h\r\nContent-Length: 0\r\n\r\n");
    CHK(response.find("405") != std::string::npos);

    server.stop();
    serverThread.join();
}

// Executes the method-not-allowed regression case.
static void run_tests() {
    RUN(reports_405_on_method_mismatch);
}

REGISTER_TEST_SUITE();
