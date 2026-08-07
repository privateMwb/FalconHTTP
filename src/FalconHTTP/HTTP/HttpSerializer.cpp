/**
 * @file HttpSerializer.cpp
 * @brief HttpSerializer implementation.
 */

// clang-format off
#include <FalconHTTP/HTTP/HttpSerializer.h> // HttpSerializer (own header)
#include <FalconHTTP/HTTP/HttpStatus.h>     // statusReasonPhrase
// clang-format on

namespace FalconHTTP::HTTP {

// Core API
std::string HttpSerializer::serialize(const HttpResponse& response) {
    std::string output;

    appendStatusLine(output, response);
    appendHeaders(output, response);
    output += response.body();

    return output;
}

// Private Helpers
void HttpSerializer::appendStatusLine(std::string& output, const HttpResponse& response) {
    output += "HTTP/1.1 ";
    output += std::to_string(static_cast<int>(response.status()));
    output += " ";
    output += statusReasonPhrase(response.status());
    output += "\r\n";
}

void HttpSerializer::appendHeaders(std::string& output, const HttpResponse& response) {
    // Header names in response.headers() are already normalized to
    // lowercase by HttpResponse::setHeader(). Content-Length and
    // Connection are computed/decided here, not by the handler, so
    // any user-set values for them are skipped to avoid emitting
    // duplicate header lines (RFC 7230 forbids duplicate
    // Content-Length; a duplicate Connection header is likewise
    // ambiguous).
    for (const auto& entry : response.headers()) {
        if (entry.key == "content-length" || entry.key == "connection") {
            continue;
        }
        output += entry.key;
        output += ": ";
        output += entry.value;
        output += "\r\n";
    }

    output += "Content-Length: ";
    output += std::to_string(response.body().size());
    output += "\r\n";

    // FalconHTTP closes every connection after one response (no
    // keep-alive support yet); HTTP/1.1 clients default to assuming
    // keep-alive unless told otherwise, so this must be explicit.
    output += "Connection: close\r\n";

    output += "\r\n";
}

} // namespace FalconHTTP::HTTP