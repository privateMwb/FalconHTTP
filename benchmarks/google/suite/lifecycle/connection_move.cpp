// FalconHTTP Connection Move Benchmark Suite
// Measures the cost of move-constructing a Connection.
//
// Server::run() moves each accepted Connection into a lambda captured
// by value before dispatching it to the thread pool
// (`pool_.detach([this, conn = std::move(connection)](){...})`).
//
// CAVEAT: the timed region below unavoidably also includes
// Socket::createTcp() - a fresh, valid, open socket is needed to move
// from on every iteration (moving from an already-moved-from source
// repeatedly would measure a degenerate empty-move case instead), and
// pre-creating enough real OS sockets ahead of time for every
// iteration at the LARGE (1M) tier isn't practical. This benchmark's
// number is therefore "create + move" together, not the move alone -
// subtract socket_construction.cpp's result from this one for an
// approximation of the move's true marginal cost.
//
// NOTE: no comparison baseline exists for FalconHTTP, so this only
// times FalconHTTP alone.
//
// Covers:
// - Connection move construction from a freshly-opened Connection

#include <FalconHTTP/FalconHTTP.h>
#include <benchmark/benchmark.h>

using namespace FalconHTTP::Core;

// Measures Connection construction + move construction together (see
// the file-level CAVEAT on why the two can't be cleanly separated).
static void bench_connection_move_construction(benchmark::State& state) {
    for (auto _ : state) {
        Connection source(Socket::createTcp());
        Connection moved(std::move(source));
        benchmark::DoNotOptimize(moved);
    }
}
BENCHMARK(bench_connection_move_construction);
