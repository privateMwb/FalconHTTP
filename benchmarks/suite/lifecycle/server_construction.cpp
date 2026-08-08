// FalconHTTP Server Construction Benchmark Suite
// Measures the cost of constructing (and destructing) a Server,
// dominated by ThreadPoolPro::ThreadPool spinning up its worker
// threads.
//
// This is a one-time-per-process-lifetime cost in real usage (a
// server is constructed once, not per-request), so this benchmark's
// absolute numbers matter less than establishing that the cost scales
// sanely with threadCount - useful context if threadCount is ever
// tuned upward significantly. It also serves as a direct check on
// ThreadPoolPro's ThreadMarket optimization (persistent worker threads
// reused across pool constructions, avoiding per-construction OS
// thread spawn/join) - if that optimization isn't actually in effect
// here, this is where it would show up as a much higher number than
// expected.
//
// CAUTION: if ThreadMarket reuse is NOT in effect, each iteration
// spawns and joins threadCount real OS threads - at the LARGE (1M)
// iteration tier that's up to 4M real thread creations, which could
// make this benchmark take dramatically longer to run than the others
// in this suite (or be impractical to run at all). Watch this one's
// wall-clock time specifically; consider excluding it from LARGE-tier
// runs if it stalls.
//
// NOTE: no comparison baseline exists for FalconHTTP (see the suite
// README) - this uses BENCH() with a single lambda, timing FalconHTTP
// alone.
//
// Covers:
// - Server construction + destruction with a 4-thread pool

#include <support/framework.h>

using namespace FalconHTTP::Core;
using namespace FalconHTTP::Routing;

namespace {
Router router;
} // namespace

// Measures Server construction and destruction with a 4-thread pool.
// The Router is constructed once outside the timed region, since this
// benchmark targets ThreadPool startup cost specifically, not Router
// construction (which is separately negligible - see Router's default
// constructor).
static void bench_server_construction() {
    auto f = [&] { Server server(router, /*threadCount=*/4); };

    BENCH("Server construction, 4 threads", f);
}

// Executes all Server construction benchmark cases.
static void run_benchmarks() {
    bench_server_construction();
}

REGISTER_BENCH_SUITE();
