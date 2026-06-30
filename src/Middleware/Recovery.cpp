#include "Middleware/Recovery.h"
#include "HTTP/HttpStatus.h"

#include <stdexcept>
#include <iostream>

namespace FalconHTTP::Middleware {

    // Core API
    void Recovery::operator()(HTTP::HttpRequest& request, HTTP::HttpResponse& response, const NextHandler& next) const {
        try {
            next(request, response);
        }
        catch (const std::exception& e) {
            std::cerr << "[ERROR] " << HTTP::methodToString(request.method()) << " "
                << request.path() << " - " << e.what() << std::endl;

            response.setStatus(HTTP::HttpStatus::InternalServerError);
            response.setBody("Internal Server Error");
        }
        catch (...) {
            std::cerr << "[ERROR] " << HTTP::methodToString(request.method()) << " "
                << request.path() << " - unknown exception" << std::endl;

            response.setStatus(HTTP::HttpStatus::InternalServerError);
            response.setBody("Internal Server Error");
        }
    }

} // namespace FalconHTTP::Middleware