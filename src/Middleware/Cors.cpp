#include "Middleware/Cors.h"
#include "HTTP/HttpStatus.h"

namespace FalconHTTP::Middleware {

    // Constructors
    Cors::Cors() noexcept
    : allowedOrigin_("*") {}

    Cors::Cors(std::string allowedOrigin)
    : allowedOrigin_(std::move(allowedOrigin)) {}

    // Core API
    void Cors::operator()(HTTP::HttpRequest& request, HTTP::HttpResponse& response, const NextHandler& next) const {
        response.setHeader("Access-Control-Allow-Origin", allowedOrigin_);
        response.setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, PATCH, OPTIONS");
        response.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");

        if (request.method() == HTTP::HttpMethod::Options) {
            response.setStatus(HTTP::HttpStatus::NoContent);
            return;
        }

        next(request, response);
    }

} // namespace FalconHTTP::Middleware
