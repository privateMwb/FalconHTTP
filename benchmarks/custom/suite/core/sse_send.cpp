// FalconHTTP SseConnection::send() Overhead Benchmark Suite
// Measures the cost of one SSE event: framing (event:/data: lines,
// per RouteHandler::send()'s multi-line convention) plus the actual
// Connection::sendAll() write, together - same as connection_move.cpp
// elsewhere in this suite, this is a real-socket cost, not an
// isolated computation, because the write is the whole point of what's
// being measured.
//
// CAVEAT: unlike every other Core benchmark, this one needs a live,
// continuously-drained peer for the full run (up to ~1.11M writes
// across the SMALL/MEDIUM/LARGE tiers) - a send() into a socket no one
// reads from would fill the kernel send buffer within a few thousand
// iterations and then block forever. A background thread on the
// client side of a real loopback connection discards everything as
// fast as it arrives for exactly this reason.
//
// NOTE: no comparison baseline exists for FalconHTTP (see the suite
// README) - this uses BENCH_SOLO() with a single lambda, timing FalconHTTP
// alone.
//
// Covers:
// - SseConnection::send() on an already-open, already-drained connection

#include <support/framework.h>

#include <atomic>
#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

using namespace FalconHTTP::Core;
using namespace FalconHTTP::Streaming;

namespace {

#ifdef _WIN32
using SocketHandle = SOCKET;

struct WinsockGuard {
    WinsockGuard() {
        WSADATA data;
        WSAStartup(MAKEWORD(2, 2), &data);
    }
    ~WinsockGuard() {
        WSACleanup();
    }
};
const WinsockGuard winsockGuard;

void closeSocket(SocketHandle fd) {
    ::closesocket(fd);
}
#else
using SocketHandle = int;

void closeSocket(SocketHandle fd) {
    ::close(fd);
}
#endif

} // namespace

// Measures SseConnection::send() end to end (framing + write) on a
// connection whose peer is continuously draining in the background.
static void bench_sse_send() {
    const uint16_t port = 18901;

    // Fully qualified: VectorPro also declares a Listener type, and
    // this file's `using namespace FalconHTTP::Core;` alone would
    // leave the name ambiguous wherever both are visible.
    FalconHTTP::Core::Listener listener;
    (void)listener.start(port);

    SocketHandle client = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    ::connect(client, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

    Connection serverSide(listener.accept());
    SseConnection sse(std::move(serverSide));

    std::atomic<bool> keepDraining{true};
    std::thread drainer([&] {
        char discard[4096];
        while (keepDraining.load(std::memory_order_relaxed)) {
            if (::recv(client, discard, sizeof(discard), 0) <= 0) {
                break;
            }
        }
    });

    auto f = [&] {
        bool ok = sse.send("tick", "the quick brown fox jumps");
        (void)ok;
    };

    BENCH_SOLO("SseConnection::send() framing + write", f);

    keepDraining.store(false, std::memory_order_relaxed);
    sse.close();
    closeSocket(client);
    drainer.join();
}

// Executes the SseConnection::send() benchmark case.
static void run_benchmarks() {
    bench_sse_send();
}

REGISTER_BENCH_SUITE();
