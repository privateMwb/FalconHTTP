#include "HTTP/HttpResponse.h"

namespace FalconHTTP::HTTP {

    // Constructors
    HttpResponse::HttpResponse(HttpStatus status) noexcept
        : status_(status) {
    }

    // Mutators
    void HttpResponse::setStatus(HttpStatus status) noexcept {
        status_ = status;
    }

    void HttpResponse::setHeader(std::string name, std::string value) {
        if (!headers_.update(name, value)) {
            headers_.insert(std::move(name), std::move(value));
        }
    }

    void HttpResponse::setBody(std::string body) noexcept {
        body_ = std::move(body);
    }

    void HttpResponse::setJson(const Json& json) {
        body_ = json.dump();
        setHeader("Content-Type", "application/json");
    }

    // Accessors
    HttpStatus HttpResponse::status() const noexcept {
        return status_;
    }

    const std::string& HttpResponse::body() const noexcept {
        return body_;
    }

    bool HttpResponse::hasHeader(const std::string& name) const noexcept {
        return headers_.contains(name);
    }

    std::string HttpResponse::header(const std::string& name) const noexcept {
        if (!hasHeader(name)) return {};
        return headers_.at(name);
    }

    const HashMap<std::string, std::string>& HttpResponse::headers() const noexcept {
        return headers_;
    }
}