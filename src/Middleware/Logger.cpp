#include "Middleware/Logger.h"

#include <iostream>
#include <chrono>

namespace FalconHTTP::Middleware {

    // Core API
    void Logger::operator()(HTTP::HttpRequest& request, HTTP::HttpResponse& response, const NextHandler& next) const {
        auto start = std::chrono::steady_clock::now();

        next(request, response);

        auto end = std::chrono::steady_clock::now();
        double durationMs = std::chrono::duration<double, std::milli>(end - start).count();

        std::cout << "[INFO] "
                  << HTTP::methodToString(request.method()) << " "
                  << request.path() << " "
                  << static_cast<int>(response.status()) << " "
                  << HTTP::statusReasonPhrase(response.status()) << " "
                  << durationMs << " ms"
                  << std::endl;
    }

} // namespace FalconHTTP::Middleware