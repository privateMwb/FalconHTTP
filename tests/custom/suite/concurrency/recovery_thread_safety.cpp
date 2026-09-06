// Recovery thread-safety concurrency test suite.
//
// Coverage:
// - Many threads triggering exceptions caught by Recovery
//   concurrently do not produce interleaved/garbled lines on stderr -
//   the same class of bug the mutex-guarded write in Recovery.cpp was
//   added to fix (see Logger's equivalent, and the note there on why
//   redirecting the stream is safe here)
// - Every thread's response still correctly becomes a 500 regardless
//   of concurrent contention

#include <support/framework.h>

// clang-format off
#include <thread>    // std::thread
#include <vector>    // std::vector
#include <sstream>   // std::ostringstream
#include <iostream>  // std::cerr
#include <stdexcept> // std::runtime_error
#include <atomic>    // std::atomic
// clang-format on

using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Middleware;

// Verifies concurrent exceptions caught by Recovery each produce one
// complete, non-corrupted error line, with none interleaved or
// truncated, and every response still ends up as 500.
static void concurrent_recovery_produces_intact_lines() {
    constexpr int threadCount = 16;
    constexpr int callsPerThread = 25;

    std::ostringstream capture;
    std::streambuf* originalBuf = std::cerr.rdbuf(capture.rdbuf());

    Recovery recovery;
    std::vector<std::thread> threads;
    std::atomic<int> non500Count{0};

    for (int t = 0; t < threadCount; ++t) {
        threads.emplace_back([&recovery, &non500Count, t]() {
            for (int i = 0; i < callsPerThread; ++i) {
                HttpRequest request;
                request.setMethod(HttpMethod::Get);
                request.setPath("/thread-" + std::to_string(t));

                HttpResponse response;

                NextHandler next = [t](HttpRequest&, HttpResponse&) {
                    throw std::runtime_error("boom-" + std::to_string(t));
                };

                recovery(request, response, next);

                if (response.status() != HttpStatus::InternalServerError) {
                    ++non500Count;
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cerr.rdbuf(originalBuf);

    CHK(non500Count.load() == 0);

    // Split captured output into lines and verify each one is intact.
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

        CHK(line.starts_with("[ERROR] GET /thread-"));
        CHK(line.find("boom-") != std::string::npos);
    }

    CHK(lineCount == static_cast<std::size_t>(threadCount * callsPerThread));
}

// Executes all Recovery thread-safety test cases.
static void run_tests() {
    RUN(concurrent_recovery_produces_intact_lines);
}

REGISTER_TEST_SUITE();
