#include "../../include/Core/Server.h"
#include "../../include/Routing/Router.h"
#include "../../include/Config/ServerConfig.h"
#include "../../include/Middleware/Logger.h"
#include "../../include/Middleware/Cors.h"
#include "../../include/Middleware/Recovery.h"
#include "../../include/HTTP/HttpResponse.h"
#include "../../include/HTTP/HttpStatus.h"

#include <iostream>
#include <csignal>

using namespace FalconHTTP;

// Signal Handling
namespace {
    Core::Server* activeServer = nullptr;

    void handleSignal(int) {
        if (activeServer != nullptr) {
            activeServer->stop();
        }
    }
}

int main() {
    Config::ServerConfig  config;
    Routing::Router       router;

    router.get("/", [](const HTTP::HttpRequest&, HTTP::HttpResponse& response) {
        response.setStatus(HTTP::HttpStatus::Ok);
        response.setBody("Hello from FalconHTTP!");
    });

    router.get("/users/:id", [](const HTTP::HttpRequest& request, HTTP::HttpResponse& response) {
        std::string id = request.pathParam("id");

        Json::ObjectType obj;
        obj["id"]       = Json(id);
        obj["message"]  = Json("User lookup placeholder");

        Json body(std::move(obj));
        response.setJson(body);
    });

    Core::Server server(router, config.threadCount);
    activeServer = &server;
    std::signal(SIGINT, handleSignal);

    server.use(Middleware::Recovery{});
    server.use(Middleware::Logger{});
    server.use(Middleware::Cors{config.corsAllowedOrigin});

    if (!server.start(config.port)) {
        std::cerr << "Failed to bind to port " << config.port << std::endl;
        return 1;
    }

    std::cout << "FalconHTTP listening on port " << config.port << std::endl;

    server.run();

    std::cout << "Server stopped." << std::endl;
    return 0;
}