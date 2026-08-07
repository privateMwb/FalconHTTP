// Sizing the connection thread pool.
//
// Demonstrates:
// - threadCount as a construction-time-only setting (no setter afterward)
// - The two ways to provide it: directly, or via ServerConfig
// - What threadCount actually bounds: FalconHTTP is thread-per-connection
//   (no async I/O), so it caps how many connections can be handled at once
//
// Note: ThreadPoolPro (the pool implementation Server uses internally) is
// an external dependency not covered by these headers - this example only
// covers the sizing knob FalconHTTP itself exposes, not ThreadPoolPro's
// own API.

#include <support/framework.h>

#include <iostream>

using namespace FalconHTTP;

static void run_examples() {

    // The threadCount-only constructor is the simplest way to pick a pool
    // size directly, independent of any other configuration.
    setTitle("Explicit Thread Count");

    Routing::Router router;
    Core::Server smallServer(router, /*threadCount=*/2);

    std::cout << "constructed with threadCount = 2\n\n";

    // ServerConfig::threadCount is the same knob, bundled with the rest of
    // an application's settings so it can be read from one shared object.
    setTitle("Via ServerConfig");

    Config::ServerConfig config;
    config.threadCount = 8;

    Core::Server configuredServer(router, config);

    std::cout << "config.threadCount : " << config.threadCount << "\n\n";

    // There's no threadCount() getter and no way to resize the pool after
    // construction - a Server is built once with the size it will use for
    // its whole lifetime. Since there's no async I/O, threadCount is
    // effectively the ceiling on simultaneous in-flight connections; a
    // request whose handler blocks (e.g. on a slow downstream call) ties
    // up one pool thread for its whole duration.
    setTitle("Sizing Guidance");

    std::cout << "pick threadCount based on:\n";
    std::cout << "  - expected concurrent connections\n";
    std::cout << "  - how long a typical handler takes to respond\n";
    std::cout << "  - available CPU cores / memory for the process\n";
}

REGISTER_EXAMPLE_SUITE();
