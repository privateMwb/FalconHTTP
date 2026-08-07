// Calling run() on a Server with no router bound.
//
// Demonstrates:
// - Server()'s default constructor leaves router_ == nullptr
// - run() checking for this and returning immediately - no crash, but
//   also no accepted connections, and no error reported anywhere
// - The same silent no-op even after start() succeeds - a bound listener
//   doesn't imply a bound router
// - The fix: use one of the parameterized constructors

#include <support/framework.h>

#include <iostream>

using namespace FalconHTTP;

static void run_examples() {

    // Server() compiles and constructs fine - there's nothing at
    // construction time that requires a router.
    setTitle("Default-Constructed Server");

    Core::Server server;

    std::cout << "isRunning() before run() : " << server.isRunning() << "\n\n";

    // start() only needs a listening socket - it has no idea whether a
    // router was ever provided, so it succeeds normally.
    setTitle("start() Still Succeeds");

    bool started = server.start(18083);
    std::cout << "start() returned : " << started << "\n\n";

    if (!started) {
        std::cout << "port 18083 unavailable, skipping the rest of this example\n";
        return;
    }

    // run() is the only place this gets caught - it checks router_ before
    // entering the accept loop and returns immediately if it's null.
    // Nothing is logged and no exception is thrown: from the caller's side
    // this looks identical to a server that started, served nothing, and
    // was asked to stop instantly.
    setTitle("run() Silently Does Nothing");

    server.run();

    std::cout << "run() returned immediately\n";
    std::cout << "isRunning() after run()  : " << server.isRunning() << "\n\n";

    // The fix is simply to use a constructor that binds a router up front.
    setTitle("The Fix");

    Routing::Router router;
    router.get("/", [](const HTTP::HttpRequest&, HTTP::HttpResponse& response) {
        response.setStatus(HTTP::HttpStatus::Ok);
        response.setBody("ok");
    });

    Core::Server workingServer(router, /*threadCount=*/2);
    std::cout << "Server(router, threadCount) has a bound router from construction\n";
}

REGISTER_EXAMPLE_SUITE();
