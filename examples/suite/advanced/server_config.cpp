// Sharing one ServerConfig across the whole application.
//
// Demonstrates:
// - ServerConfig's defaults
// - Overriding individual fields
// - Which fields Server itself reads (port, threadCount, maxHeaderSize,
//   maxBodySize) vs. which it doesn't (staticRoot, fileCacheCapacity,
//   corsAllowedOrigin) - those exist purely so an application can build
//   StaticFileServer and Cors from the same config object
// - Config is read once at Server construction time - mutating it
//   afterward has no effect on an already-built Server

#include <support/framework.h>

#include <iostream>

using namespace FalconHTTP;

static void run_examples() {

    // A default-constructed ServerConfig is ready to use as-is.
    setTitle("Defaults");

    Config::ServerConfig config;

    std::cout << "port              : " << config.port << "\n";
    std::cout << "threadCount       : " << config.threadCount << "\n";
    std::cout << "maxHeaderSize     : " << config.maxHeaderSize << " bytes\n";
    std::cout << "maxBodySize       : " << config.maxBodySize << " bytes\n";
    std::cout << "staticRoot        : " << config.staticRoot << "\n";
    std::cout << "fileCacheCapacity : " << config.fileCacheCapacity << "\n";
    std::cout << "corsAllowedOrigin : " << config.corsAllowedOrigin << "\n\n";

    // Fields are plain public data - override whichever ones an
    // application needs before handing the config to anything.
    setTitle("Overriding Fields");

    config.port = 9090;
    config.maxBodySize = 1 * 1024 * 1024; // 1 MiB, stricter than the 10 MiB default
    config.corsAllowedOrigin = "https://example.com";

    std::cout << "port              : " << config.port << "\n";
    std::cout << "maxBodySize       : " << config.maxBodySize << " bytes\n";
    std::cout << "corsAllowedOrigin : " << config.corsAllowedOrigin << "\n\n";

    // Server(Router&, const ServerConfig&) only reads port, threadCount,
    // maxHeaderSize, and maxBodySize. staticRoot, fileCacheCapacity, and
    // corsAllowedOrigin pass straight through - Server doesn't own a
    // StaticFileServer or Cors instance, so those pieces are built
    // separately from the same shared config.
    setTitle("One Config, Three Objects");

    Routing::Router router;
    Core::Server server(router, config);

    FileServer::StaticFileServer staticFiles(config.staticRoot, config.fileCacheCapacity);
    Middleware::Cors cors(config.corsAllowedOrigin);

    std::cout << "Server, StaticFileServer, and Cors all built from one ServerConfig\n\n";

    // config is read once at construction - changing it afterward does
    // not reach back into the Server already built from it.
    setTitle("Config Is Read Once");

    config.port = 3000;
    std::cout << "config.port changed to 3000, but server keeps the port it was built with\n";
}

REGISTER_EXAMPLE_SUITE();
