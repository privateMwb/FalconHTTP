/**
 * @file HttpResponse.cpp
 * @brief HttpResponse implementation.
 *
 * Contains the implementation of HttpResponse's construction, mutators,
 * and accessors.
 */

// ============================================================
// Implementation for FalconHTTP::HTTP::HttpResponse.
// ============================================================
//
//  Sections:
//   1. Constructors
//   2. Mutators
//   3. Accessors
//
// ============================================================

#include <FalconHTTP/HTTP/HttpResponse.h> // HttpResponse (own header)

// clang-format off
#include <algorithm> // std::transform
#include <cctype>    // std::tolower
// clang-format on

namespace FalconHTTP::HTTP {

namespace {
// Header field names are case-insensitive per RFC 7230 §3.2.
// Normalizing to lowercase on both insert and lookup gives
// case-insensitive semantics without a case-insensitive HashMap.
// Note: this also means header names are emitted on the wire in
// lowercase (HttpSerializer writes headers_ keys verbatim) -
// this is spec-legal (RFC 7230 §3.2, mandatory in HTTP/2) but is
// a visible behavior change from mixed-case output.
std::string toLowerAscii(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}
} // namespace

// ============================================================
//  Section 1 — Constructors
// ============================================================

HttpResponse::HttpResponse(HttpStatus status) noexcept : status_(status) {}

// ============================================================
//  Section 2 — Mutators
// ============================================================

void HttpResponse::setStatus(HttpStatus status) noexcept {
    status_ = status;
}

void HttpResponse::setHeader(std::string name, std::string value) {
    std::string key = toLowerAscii(std::move(name));
    if (!headers_.update(key, value)) {
        headers_.insert(std::move(key), std::move(value));
    }
}

void HttpResponse::setBody(std::string body) noexcept {
    body_ = std::move(body);
}

void HttpResponse::setJson(const Json& json) {
    body_ = json.dump();
    setHeader("Content-Type", "application/json");
}

// ============================================================
//  Section 3 — Accessors
// ============================================================

HttpStatus HttpResponse::status() const noexcept {
    return status_;
}

const std::string& HttpResponse::body() const noexcept {
    return body_;
}

bool HttpResponse::hasHeader(const std::string& name) const noexcept {
    return headers_.contains(toLowerAscii(name));
}

std::string HttpResponse::header(const std::string& name) const noexcept {
    std::string key = toLowerAscii(name);
    if (!headers_.contains(key))
        return {};
    return headers_.at(key);
}

const HashMap<std::string, std::string>& HttpResponse::headers() const noexcept {
    return headers_;
}

} // namespace FalconHTTP::HTTP