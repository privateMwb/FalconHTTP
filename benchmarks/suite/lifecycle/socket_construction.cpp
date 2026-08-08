// FalconHTTP Socket Construction Benchmark Suite
// Measures the cost of creating and closing a TCP socket via
// Socket::createTcp().
//
// This is a real syscall-backed operation (socket()/close()), not a
// pure in-process allocation - it establishes the floor cost every
// accepted connection pays before any I/O happens, and the cost
// Listener::accept() and Connection construction build on top of.
//
// NOTE: no comparison baseline exists for FalconHTTP (see the suite
// README) - this uses BENCH() with a single lambda, timing FalconHTTP
// alone.
//
// Covers:
// - Socket::createTcp() construction followed by close()

#include <support/framework.h>

using namespace FalconHTTP::Core;

// Measures createTcp() + close() as one paired operation, since a
// socket left open would exhaust file descriptors across iterations.
static void bench_socket_construction() {
    auto f = [&] {
        Socket socket = Socket::createTcp();
        socket.close();
    };

    BENCH("Socket::createTcp() + close()", f);
}

// Executes all socket construction benchmark cases.
static void run_benchmarks() {
    bench_socket_construction();
}

REGISTER_BENCH_SUITE();
