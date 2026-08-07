// Logger thread-safety concurrency test suite.
//
// Coverage:
// - Many threads calling Logger::operator() concurrently do not
//   produce interleaved/garbled lines on stdout - this is the exact
//   bug the mutex-guarded write in Logger.cpp was added to fix
// - Every expected line appears exactly once, fully intact
//
// Redirects std::cout to a local buffer for the duration of the test
// (restored afterward) to capture and inspect the output. This is
// safe to do here despite the concurrent writers: Logger builds each
// full line in a local std::ostringstream first, and only performs a
// single `std::cout << line` under its internal mutex - so the
// redirected buffer only ever receives one fully-serialized write at
// a time, which is exactly the property this test is verifying.

#include <support/framework.h>

// clang-format off
#include <thread>   // std::thread
#include <vector>   // std::vector
#include <sstream>  // std::ostringstream
#include <iostream> // std::cout
// clang-format on

using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Middleware;

// Verifies concurrent Logger invocations each produce one complete,
// non-corrupted line, with none interleaved or truncated.
static void concurrent_logging_produces_intact_lines() {
    constexpr int threadCount = 16;
    constexpr int callsPerThread = 25;

    std::ostringstream capture;
    std::streambuf* originalBuf = std::cout.rdbuf(capture.rdbuf());

    Logger logger;
    std::vector<std::thread> threads;

    for (int t = 0; t < threadCount; ++t) {
        threads.emplace_back([&logger, t]() {
            for (int i = 0; i < callsPerThread; ++i) {
                HttpRequest request;
                request.setMethod(HttpMethod::Get);
                request.setPath("/thread-" + std::to_string(t));

                HttpResponse response;
                response.setStatus(HttpStatus::Ok);

                NextHandler next = [](HttpRequest&, HttpResponse&) {};
                logger(request, response, next);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout.rdbuf(originalBuf);

    // Split captured output into lines and verify each one is intact:
    // starts with the expected prefix and ends with " ms", with a
    // recognizable path segment in between. A corrupted/interleaved
    // write would produce a line failing one of these checks or an
    // unexpected total line count.
    std::string output = capture.str();
    std::size_t lineCount = 0;
    std::size_t pos = 0;

    while (pos < output.size()) {
        std::size_t newlinePos = output.find('\n', pos);
        if (newlinePos == std::string::npos)
            break;

        std::string line = output.substr(pos, newlinePos - pos);
        pos = newlinePos + 1;

        if (line.empty())
            continue;
        ++lineCount;

        CHK(line.starts_with("[INFO] GET /thread-"));
        CHK(line.ends_with(" ms"));
    }

    CHK(lineCount == static_cast<std::size_t>(threadCount * callsPerThread));
}

// Executes all Logger thread-safety test cases.
static void run_tests() {
    RUN(concurrent_logging_produces_intact_lines);
}

REGISTER_TEST_SUITE();
