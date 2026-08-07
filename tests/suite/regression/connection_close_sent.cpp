// Regression test: every response must include "Connection: close",
// since FalconHTTP does not implement keep-alive and closes every
// connection after exactly one response.

#include <support/framework.h>

// clang-format off
#include <thread>
#include <chrono>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
// clang-format on

using namespace FalconHTTP::Core;
using namespace FalconHTTP::Routing;
using namespace FalconHTTP::HTTP;

namespace {

constexpr uint16_t TestPort = 18473;

std::string sendRawRequest(uint16_t port, const std::string& request) {
    Socket socket = Socket::createTcp();
    if (!socket.isValid())
        return {};

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    ::inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (::connect(socket.handle(),
                  reinterpret_cast<sockaddr*>(&addr),
                  sizeof(addr)) != 0) {
        return {};
    }

    if (socket.send(request.data(), request.size()) < 0)
        return {};

    std::string response;
    char buffer[4096];

    for (;;) {
        std::ptrdiff_t n = socket.receive(buffer, sizeof(buffer));

        if (n <= 0)
            break;

        response.append(buffer, static_cast<std::size_t>(n));
    }

    return response;
}

} // namespace

static void live_response_includes_connection_close() {
    Router router;
    router.get("/health", [](const HttpRequest&, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
        response.setBody("ok");
    });

    Server server(router, 2);
    CHK(server.start(TestPort));

    std::thread runner([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    std::string response =
        sendRawRequest(TestPort,
                       "GET /health HTTP/1.1\r\n"
                       "Host: h\r\n"
                       "\r\n");

    server.stop();
    runner.join();

    CHK(response.find("Connection: close\r\n") != std::string::npos);
    CHK(response.starts_with("HTTP/1.1 200 OK\r\n"));
    CHK(response.ends_with("ok"));
}

static void run_tests() {
    RUN(live_response_includes_connection_close);
}

REGISTER_TEST_SUITE();