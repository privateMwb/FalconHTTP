#include "HTTP/HttpRequest.h"

namespace FalconHTTP::HTTP {

    // Mutators
    void HttpRequest::setMethod(HttpMethod method) noexcept {
        method_ = method;
    }

    void HttpRequest::setPath(std::string path) noexcept {
        path_ = path;
    }

    void HttpRequest::setVersion(std::string version) noexcept {
        version_ = std::move(version);
    }

    void HttpRequest::setHeader(std::string name, std::string value) {
        if (!headers_.update(name, value)) {
            headers_.insert(std::move(name), std::move(value));
        }
    }

    void HttpRequest::setQueryParam(std::string name, std::string value) {
        if (!queryParams_.update(name, value)) {
            queryParams_.insert(std::move(name), std::move(value));
        }
    }

    void HttpRequest::setPathParam(std::string name, std::string value) {
        if (!pathParams_.update(name, value)) {
            pathParams_.insert(std::move(name), std::move(value));
        }
    }

    void HttpRequest::setBody(std::string body) noexcept {
        body_ = std::move(body);
    }

    // Accessors
    HttpMethod HttpRequest::method() const noexcept {
        return method_;
    }

    const std::string& HttpRequest::path() const noexcept {
        return path_;
    }

    const std::string& HttpRequest::version() const noexcept {
        return version_;
    }

    const std::string& HttpRequest::body() const noexcept {
        return body_;
    }

    bool HttpRequest::hasHeader(const std::string& name) const noexcept {
        return headers_.contains(name);
    }

    std::string HttpRequest::header(const std::string& name) const noexcept {
        if (!hasHeader(name)) return {};
        return headers_.at(name);
    }

    bool HttpRequest::hasQueryParam(const std::string& name) const noexcept {
        return queryParams_.contains(name);
    }

    std::string HttpRequest::queryParam(const std::string& name) const noexcept {
        if (!hasQueryParam(name)) return {};
        return queryParams_.at(name);
    }

    bool HttpRequest::hasPathParam(const std::string& name) const noexcept {
        return pathParams_.contains(name);
    }

    std::string HttpRequest::pathParam(const std::string& name) const noexcept {
        if (!hasPathParam(name)) return {};
        return pathParams_.at(name);
    }

    // JSON
    Json HttpRequest::json() const {
        return Json::parse(body_);
    }

} // namespace FalconHTTP::HTTP