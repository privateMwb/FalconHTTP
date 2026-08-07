// Starting, running, and stopping a Server.
//
// Demonstrates:
// - isRunning() before run() is called, while it's blocked in run(), and
//   after run() returns
// - Calling stop() from another thread to unblock a blocking run() call
// - What stop() actually does: it closes the listening socket, which is
//   what unblocks the accept() call run()'s loop is parked in
// - stop() does NOT forcibly interrupt a connection already being handled
//   on the thread pool - it only stops accepting new ones

#include <support/framework.h>

#include <chrono>
#include <iostream>
#include <thread>

using namespace FalconHTTP;

static void run_examples() {

    // Before start()/run(), isRunning() is false - a default-constructed
    // Server has never entered its accept loop.
    setTitle("Before Starting");

    Routing::Router router;
    router.get("/", [](const HTTP::HttpRequest&, HTTP::HttpResponse& response) {
        response.setStatus(HTTP::HttpStatus::Ok);
        response.setBody("ok");
    });

    Core::Server server(router, /*threadCount=*/2);

    std::cout << "isRunning() : " << server.isRunning() << "\n\n";

    // start() only binds and listens - it doesn't block. run() is what
    // enters the accept loop and blocks until stop() is called.
    setTitle("Starting and Running");

    if (!server.start(18080)) {
        std::cout << "port 18080 unavailable, skipping the rest of this example\n";
        return;
    }

    // Since run() blocks the calling thread, stop() has to come from
    // somewhere else - here, a background thread that waits briefly (so
    // run()'s loop is actually inside accept()) and then stops the server.
    std::thread stopper([&server]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "isRunning() while active : " << server.isRunning() << "\n";
        server.stop();
    });

    server.run(); // blocks here until stopper's server.stop() call unblocks accept()
    stopper.join();

    std::cout << "run() returned\n";
    std::cout << "isRunning() after stop()  : " << server.isRunning() << "\n";
}

REGISTER_EXAMPLE_SUITE();
