// Starting a FalconHTTP server.
//
// Demonstrates:
// - Registering GET routes, including one with a path param
// - Returning a plain-text body and a JSON body
// - Constructing a Server from a ServerConfig
// - Registering Recovery, Logger, and Cors middleware
// - Starting the server and blocking in run() until interrupted (Ctrl+C)

#include <support/framework.h>

#include <csignal>
#include <iostream>

using namespace FalconHTTP;

// Signal Handling
namespace {
Core::Server* activeServer = nullptr;

void handleSignal(int) {
    if (activeServer != nullptr) {
        activeServer->stop();
    }
}
} // namespace

static void run_examples() {

    // Routes are registered on a Router before it's handed to a Server.
    // A `:name` segment binds the matching path segment as a path param.
    setTitle("Registering Routes");

    Routing::Router router;

    router.get("/", [](const HTTP::HttpRequest&, HTTP::HttpResponse& response) {
        response.setStatus(HTTP::HttpStatus::Ok);
        response.setBody("Hello from FalconHTTP!\n");
    });

    router.get("/users/:id", [](const HTTP::HttpRequest& request, HTTP::HttpResponse& response) {
        std::string id = request.pathParam("id");

        Json::ObjectType obj;
        obj["id"] = Json(id);
        obj["message"] = Json("User lookup placeholder");

        response.setJson(Json(std::move(obj)));
    });

    std::cout << "routes registered: GET /, GET /users/:id\n\n";

    // Server(Router&, const ServerConfig&) reads threadCount, port,
    // maxHeaderSize, and maxBodySize from the config in one call.
    setTitle("Constructing the Server");

    Config::ServerConfig config;
    Core::Server server(router, config);

    activeServer = &server;
    std::signal(SIGINT, handleSignal);

    std::cout << "thread count : " << config.threadCount << "\n";
    std::cout << "port         : " << config.port << "\n\n";

    // Middleware runs in registration order, onion-model: Recovery should
    // be registered first so it can catch exceptions from everything after
    // it in the chain, including Logger, Cors, and the route handlers.
    setTitle("Registering Middleware");

    server.use(Middleware::Recovery{});
    server.use(Middleware::Logger{});
    server.use(Middleware::Cors{config.corsAllowedOrigin});

    std::cout << "middleware registered: Recovery, Logger, Cors\n\n";

    // start() binds the listening socket; run() blocks, accepting and
    // handling connections until stop() is called from another thread -
    // here, from the SIGINT handler above.
    setTitle("Starting the Server");

    if (!server.start()) {
        std::cerr << "Failed to bind to port " << config.port << std::endl;
        return;
    }

    std::cout << "FalconHTTP listening on port " << config.port << std::endl;

    server.run();

    std::cout << "Server stopped.\n";
}

REGISTER_EXAMPLE_SUITE();
