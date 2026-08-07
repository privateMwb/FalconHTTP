// Server start/run/stop lifecycle test suite.
//
// Coverage:
// - A default-constructed Server is not running
// - run() on a default-constructed Server (no router bound) is a
//   no-op - it returns immediately rather than blocking
// - start() binds the listener successfully
// - run()/stop() correctly transition isRunning() when driven from a
//   background thread, and stop() causes a blocked run() to return
//
// NOTE: the last test relies on Listener::stop() (called by
// Server::stop()) unblocking a concurrent thread parked in
// Listener::accept(). This is common practice but is not guaranteed
// portable POSIX behavior in general (closing a file descriptor while
// another thread is blocked in a syscall on it is platform-dependent);
// it is exercised here because it's exactly what Server::stop() relies
// on in production, so if it's flaky, that's a real finding about
// Server's shutdown path, not just a test artifact.

#include <support/framework.h>

// clang-format off
#include <thread>  // std::thread
#include <chrono>  // std::chrono::milliseconds
// clang-format on

using namespace FalconHTTP::Core;
using namespace FalconHTTP::Routing;

// Verifies a default-constructed Server is not running.
static void default_server_is_not_running() {
    Server server;
    CHK(!server.isRunning());
}

// Verifies run() on a Server with no router bound returns immediately
// rather than blocking, and does not flip isRunning() to true.
static void run_is_a_noop_without_router() {
    Server server;
    server.run();
    CHK(!server.isRunning());
}

// Verifies start() successfully binds the listener on an ephemeral
// port (0), independent of run()/stop().
static void start_binds_the_listener() {
    Router router;
    Server server(router, /*threadCount=*/2);

    CHK(server.start(0));
}

// Verifies run() flips isRunning() to true while active on a
// background thread, and stop() flips it back to false and causes
// run() to return so the thread can be joined.
static void run_and_stop_transition_is_running() {
    Router router;
    Server server(router, /*threadCount=*/2);
    CHK(server.start(0));

    std::thread runner([&server]() { server.run(); });

    // Give run()'s loop a moment to pass its guard checks and set
    // running_ = true before we observe it.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    CHK(server.isRunning());

    server.stop();
    runner.join();

    CHK(!server.isRunning());
}

// Executes all Server lifecycle test cases.
static void run_tests() {
    RUN(default_server_is_not_running);
    RUN(run_is_a_noop_without_router);
    RUN(start_binds_the_listener);
    RUN(run_and_stop_transition_is_running);
}

REGISTER_TEST_SUITE();
