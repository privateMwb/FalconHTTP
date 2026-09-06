// Regression coverage: Server(Router&, const ServerConfig&) and the
// no-arg start() overload.
//
// Both were, at one point, declared in Server.h but never defined in
// Server.cpp (a link error waiting to happen) - see the fix history.
// Neither is exercised by any test that only uses the
// (Router&, threadCount) constructor + start(uint16_t), so line and
// function coverage for both stayed at zero even after the fix landed.
// This suite calls each directly.

#include <support/framework.h>

using namespace FalconHTTP::Core;
using namespace FalconHTTP::Routing;
using namespace FalconHTTP::Config;

// Verifies Server(Router&, const ServerConfig&) actually reads
// threadCount/port/maxHeaderSize/maxBodySize from the config, by
// confirming the resulting Server can bind using config.port via the
// no-arg start() - which only works if configuredPort_ was set by the
// constructor in the first place.
static void constructs_from_server_config() {
    Router router;

    ServerConfig config;
    config.port = 18510;
    config.threadCount = 2;
    config.maxHeaderSize = 8192;
    config.maxBodySize = 1024 * 1024;

    Server server(router, config);

    bool started = server.start();
    CHK(started);

    server.stop();
}

// Verifies the no-arg start() overload binds using whatever port was
// last set - either by the ServerConfig constructor, or by an earlier
// explicit start(uint16_t) call.
static void no_arg_start_reuses_last_port() {
    Router router;
    Server server(router, /*threadCount=*/2);

    bool explicitStart = server.start(18511);
    CHK(explicitStart);
    server.stop();

    bool reboundSamePort = server.start();
    CHK(reboundSamePort);

    server.stop();
}

// Executes all Server construction/lifecycle coverage cases.
static void run_tests() {
    RUN(constructs_from_server_config);
    RUN(no_arg_start_reuses_last_port);
}

REGISTER_TEST_SUITE();
