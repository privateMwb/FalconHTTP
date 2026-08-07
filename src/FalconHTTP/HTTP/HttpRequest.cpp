/**
 * @file HttpRequest.cpp
 * @brief HttpRequest implementation.
 *
 * Contains the implementation of HttpRequest's mutators, accessors, and
 * JSON body parsing.
 */

// ============================================================
// Implementation for FalconHTTP::HTTP::HttpRequest.
// ============================================================
//
//  Sections:
//   1. Mutators
//   2. Accessors
//   3. JSON
//
// ============================================================

#include <FalconHTTP/HTTP/HttpRequest.h> // HttpRequest (own header)

// clang-format off
#include <algorithm> // std::transform
#include <cctype>    // std::tolower
// clang-format on

namespace FalconHTTP::HTTP {

namespace {
// Header field names are case-insensitive per RFC 7230 §3.2.
// Normalizing to lowercase on both insert and lookup gives
// case-insensitive semantics without a case-insensitive HashMapPro.
std::string toLowerAscii(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}
} // namespace

// ============================================================
//  Section 1 — Mutators
// ============================================================

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
    std::string key = toLowerAscii(std::move(name));
    if (!headers_.update(key, value)) {
        headers_.insert(std::move(key), std::move(value));
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

// ============================================================
//  Section 2 — Accessors
// ============================================================

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
    return headers_.contains(toLowerAscii(name));
}

std::string HttpRequest::header(const std::string& name) const noexcept {
    std::string key = toLowerAscii(name);
    if (!headers_.contains(key))
        return {};
    return headers_.at(key);
}

bool HttpRequest::hasQueryParam(const std::string& name) const noexcept {
    return queryParams_.contains(name);
}

std::string HttpRequest::queryParam(const std::string& name) const noexcept {
    if (!hasQueryParam(name))
        return {};
    return queryParams_.at(name);
}

bool HttpRequest::hasPathParam(const std::string& name) const noexcept {
    return pathParams_.contains(name);
}

std::string HttpRequest::pathParam(const std::string& name) const noexcept {
    if (!hasPathParam(name))
        return {};
    return pathParams_.at(name);
}

// ============================================================
//  Section 3 — JSON
// ============================================================

Json HttpRequest::json() const {
    return Json::parse(body_);
}

} // namespace FalconHTTP::HTTP