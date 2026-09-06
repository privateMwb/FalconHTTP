// FalconHTTP Socket Construction Benchmark Suite
// Measures the cost of creating and closing a TCP socket via
// Socket::createTcp().
//
// This is a real syscall-backed operation (socket()/close()), not a
// pure in-process allocation - it establishes the floor cost every
// accepted connection pays before any I/O happens, and the cost
// Listener::accept() and Connection construction build on top of.
//
// NOTE: no comparison baseline exists for FalconHTTP, so this only
// times FalconHTTP alone.
//
// Covers:
// - Socket::createTcp() construction followed by close()

#include <FalconHTTP/FalconHTTP.h>
#include <benchmark/benchmark.h>

using namespace FalconHTTP::Core;

// Measures createTcp() + close() as one paired operation, since a
// socket left open would exhaust file descriptors across iterations.
static void bench_socket_construction(benchmark::State& state) {
    for (auto _ : state) {
        Socket socket = Socket::createTcp();
        socket.close();
        benchmark::DoNotOptimize(socket);
    }
}
BENCHMARK(bench_socket_construction);
