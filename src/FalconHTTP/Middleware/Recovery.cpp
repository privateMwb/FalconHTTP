/**
 * @file Recovery.cpp
 * @brief Recovery middleware implementation.
 */

// clang-format off
#include <FalconHTTP/Middleware/Recovery.h> // Recovery (own header)
#include <FalconHTTP/HTTP/HttpStatus.h>     // HttpStatus

#include <stdexcept> // std::exception
#include <iostream>  // std::cerr
#include <sstream>   // std::ostringstream
#include <mutex>     // std::mutex, std::lock_guard
// clang-format on

namespace FalconHTTP::Middleware {

namespace {
// Same rationale as Logger.cpp: Server dispatches requests onto a
// thread pool, so concurrent Recovery invocations would otherwise
// interleave/garble std::cerr output.
std::mutex& logMutex() {
    static std::mutex mutex;
    return mutex;
}

void writeErrorLine(const HTTP::HttpRequest& request, const std::string& message) {
    std::ostringstream line;
    line << "[ERROR] " << HTTP::methodToString(request.method()) << " " << request.path() << " - "
         << message << "\n";

    std::lock_guard<std::mutex> lock(logMutex());
    std::cerr << line.str() << std::flush;
}
} // namespace

// Core API
void Recovery::operator()(HTTP::HttpRequest& request, HTTP::HttpResponse& response,
                          const NextHandler& next) const {
    try {
        next(request, response);
    } catch (const std::exception& e) {
        writeErrorLine(request, e.what());

        response.setStatus(HTTP::HttpStatus::InternalServerError);
        response.setBody("Internal Server Error");
    } catch (...) {
        writeErrorLine(request, "unknown exception");

        response.setStatus(HTTP::HttpStatus::InternalServerError);
        response.setBody("Internal Server Error");
    }
}

} // namespace FalconHTTP::Middleware