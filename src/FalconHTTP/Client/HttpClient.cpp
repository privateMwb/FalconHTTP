/**
 * @file HttpClient.cpp
 * @brief HttpClient implementation.
 *
 * Contains the implementation of HttpClient's request building,
 * response reading, and response parsing.
 */

// ============================================================
// Implementation for FalconHTTP::Client::HttpClient.
// ============================================================
//
//  Sections:
//   1. Request Building & Response Parsing Helpers
//   2. Construction
//   3. Sending
//
// ============================================================

// clang-format off
#include <FalconHTTP/Client/HttpClient.h> // HttpClient — the class this file implements

#include <FalconHTTP/Core/Socket.h> // Socket — connect(), timeouts, send(), receive()

#include <algorithm>    // std::transform - lowercasing header names
#include <array>        // std::array - receive buffer
#include <cctype>       // std::tolower - header-name normalization
#include <charconv>     // std::from_chars - status code and Content-Length parsing
#include <string_view>  // std::string_view - zero-copy header parsing
// clang-format on

namespace FalconHTTP::Client {

// ============================================================
//  Section 1 — Request Building & Response Parsing Helpers
// ============================================================

namespace {

std::string lowercase(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

std::string_view trim(std::string_view text) {
    while (!text.empty() && (text.front() == ' ' || text.front() == '\t'))
        text.remove_prefix(1);
    while (!text.empty() && (text.back() == ' ' || text.back() == '\t' || text.back() == '\r'))
        text.remove_suffix(1);
    return text;
}

bool parseNumber(std::string_view text, std::size_t& out) {
    text = trim(text);
    if (text.empty())
        return false;

    const auto result = std::from_chars(text.data(), text.data() + text.size(), out);
    return result.ec == std::errc() && result.ptr == text.data() + text.size();
}

std::string buildRequest(HTTP::HttpMethod method, const std::string& host, uint16_t port,
                         const std::string& path, const std::string& body,
                         const HashMapPro::HashMap<std::string, std::string>& headers) {
    std::string request;
    request += HTTP::methodToString(method);
    request += ' ';
    request += path;
    request += " HTTP/1.1\r\nHost: ";
    request += host;
    request += ':';
    request += std::to_string(port);
    request += "\r\nConnection: close\r\nContent-Length: ";
    request += std::to_string(body.size());
    request += "\r\n";

    for (const auto& entry : headers) {
        const std::string name = lowercase(entry.key);
        if (name == "host" || name == "connection" || name == "content-length")
            continue; // The client owns these three.

        request += entry.key;
        request += ": ";
        request += entry.value;
        request += "\r\n";
    }

    request += "\r\n";
    request += body;
    return request;
}

// Parses the status line and header block (everything before the blank
// line) into @p out. @return false if either is malformed.
bool parseHead(std::string_view head, ClientResponse& out) {
    const std::size_t lineEnd = head.find("\r\n");
    const std::string_view statusLine = head.substr(0, lineEnd);

    // "HTTP/1.1 200 OK": the code is the second space-separated token.
    const std::size_t firstSpace = statusLine.find(' ');
    if (firstSpace == std::string_view::npos || !statusLine.starts_with("HTTP/"))
        return false;

    const std::string_view afterVersion = statusLine.substr(firstSpace + 1);
    std::size_t code = 0;
    if (!parseNumber(afterVersion.substr(0, afterVersion.find(' ')), code) || code < 100 ||
        code > 599)
        return false;
    out.statusCode = static_cast<int>(code);

    std::string_view rest =
        lineEnd == std::string_view::npos ? std::string_view() : head.substr(lineEnd + 2);
    while (!rest.empty()) {
        const std::size_t next = rest.find("\r\n");
        const std::string_view line = rest.substr(0, next);
        rest = next == std::string_view::npos ? std::string_view() : rest.substr(next + 2);

        const std::size_t colon = line.find(':');
        if (colon == std::string_view::npos)
            return false;

        out.headers[lowercase(std::string(trim(line.substr(0, colon))))] =
            std::string(trim(line.substr(colon + 1)));
    }

    return true;
}

} // namespace

// ============================================================
//  Section 2 — Construction
// ============================================================

HttpClient::HttpClient(ClientOptions options) noexcept : options_(options) {}

// ============================================================
//  Section 3 — Sending
// ============================================================

std::optional<ClientResponse>
HttpClient::send(HTTP::HttpMethod method, const std::string& host, uint16_t port,
                 const std::string& path, const std::string& body,
                 const HashMapPro::HashMap<std::string, std::string>& headers) const {
    Core::Socket socket = Core::Socket::createTcp();

    if (!socket.isValid() || !socket.connect(host, port, options_.connectTimeout) ||
        !socket.setSendTimeout(options_.ioTimeout) || !socket.setReceiveTimeout(options_.ioTimeout))
        return std::nullopt;

    (void)socket.setNoDelay(true); // Best effort: small RPCs shouldn't wait on Nagle.

    const std::string request = buildRequest(method, host, port, path, body, headers);
    std::size_t sent = 0;
    while (sent < request.size()) {
        const std::ptrdiff_t written = socket.send(request.data() + sent, request.size() - sent);
        if (written <= 0)
            return std::nullopt;
        sent += static_cast<std::size_t>(written);
    }

    // Read until the response is complete: Content-Length bytes of body
    // once the head is parsed, or EOF when the server sent no length.
    ClientResponse response;
    std::string received;
    std::size_t headEnd = std::string::npos;
    std::size_t expectedBody = 0;
    bool hasLength = false;
    std::array<char, 4096> chunk;

    while (true) {
        if (headEnd != std::string::npos && hasLength && received.size() - headEnd >= expectedBody)
            break;

        const std::ptrdiff_t got = socket.receive(chunk.data(), chunk.size());
        if (got < 0)
            return std::nullopt; // Error or timeout.
        if (got == 0) {
            // EOF: only a complete response if the head arrived and
            // either no length was promised or it was fully delivered.
            if (headEnd == std::string::npos)
                return std::nullopt;
            if (hasLength && received.size() - headEnd < expectedBody)
                return std::nullopt;
            break;
        }

        received.append(chunk.data(), static_cast<std::size_t>(got));
        if (received.size() > options_.maxResponseSize)
            return std::nullopt;

        if (headEnd == std::string::npos) {
            const std::size_t blank = received.find("\r\n\r\n");
            if (blank == std::string::npos)
                continue;

            if (!parseHead(std::string_view(received).substr(0, blank), response))
                return std::nullopt;

            headEnd = blank + 4;
            if (response.headers.contains("content-length")) {
                if (!parseNumber(response.headers["content-length"], expectedBody) ||
                    expectedBody > options_.maxResponseSize)
                    return std::nullopt;
                hasLength = true;
            }
        }
    }

    response.body = received.substr(headEnd, hasLength ? expectedBody : std::string::npos);
    return response;
}

std::optional<ClientResponse> HttpClient::get(const std::string& host, uint16_t port,
                                              const std::string& path) const {
    return send(HTTP::HttpMethod::Get, host, port, path);
}

std::optional<ClientResponse>
HttpClient::post(const std::string& host, uint16_t port, const std::string& path,
                 const std::string& body,
                 const HashMapPro::HashMap<std::string, std::string>& headers) const {
    return send(HTTP::HttpMethod::Post, host, port, path, body, headers);
}

std::optional<ClientResponse>
HttpClient::put(const std::string& host, uint16_t port, const std::string& path,
                const std::string& body,
                const HashMapPro::HashMap<std::string, std::string>& headers) const {
    return send(HTTP::HttpMethod::Put, host, port, path, body, headers);
}

std::optional<ClientResponse> HttpClient::del(const std::string& host, uint16_t port,
                                              const std::string& path) const {
    return send(HTTP::HttpMethod::Delete, host, port, path);
}

} // namespace FalconHTTP::Client
