/**
 * @file Logger.cpp
 * @brief Logger middleware implementation.
 */

// clang-format off
#include <FalconHTTP/Middleware/Logger.h> // Logger (own header)

#include <iostream> // std::cout
#include <sstream>  // std::ostringstream
#include <chrono>   // std::chrono::steady_clock
#include <mutex>    // std::mutex, std::lock_guard
// clang-format on

namespace FalconHTTP::Middleware {

namespace {
// Server dispatches requests onto a thread pool, so concurrent
// Logger invocations would otherwise interleave/garble std::cout
// output. Build the line in a local buffer and emit it under a
// single lock so each log line is written atomically.
std::mutex& logMutex() {
    static std::mutex mutex;
    return mutex;
}
} // namespace

// Core API
void Logger::operator()(HTTP::HttpRequest& request, HTTP::HttpResponse& response,
                        const NextHandler& next) const {
    auto start = std::chrono::steady_clock::now();

    next(request, response);

    auto end = std::chrono::steady_clock::now();
    double durationMs = std::chrono::duration<double, std::milli>(end - start).count();

    std::ostringstream line;
    line << "[INFO] " << HTTP::methodToString(request.method()) << " " << request.path() << " "
         << static_cast<int>(response.status()) << " "
         << HTTP::statusReasonPhrase(response.status()) << " " << durationMs << " ms\n";

    std::lock_guard<std::mutex> lock(logMutex());
    std::cout << line.str() << std::flush;
}

} // namespace FalconHTTP::Middleware