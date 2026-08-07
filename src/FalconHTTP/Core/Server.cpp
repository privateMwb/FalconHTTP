/**
 * @file Server.cpp
 * @brief Server implementation.
 *
 * Contains the implementation of Server's lifecycle, middleware
 * registration, and per-connection request handling.
 */

// ============================================================
// Implementation for FalconHTTP::Core::Server.
// ============================================================
//
//  Sections:
//   1. Constructor
//   2. Middleware Registration
//   3. Lifecycle
//   4. Private Helpers
//
// ============================================================

// clang-format off
#include <FalconHTTP/Core/Server.h>          // Server (own header)
#include <FalconHTTP/HTTP/HttpParser.h>      // HttpParser::parse
#include <FalconHTTP/HTTP/HttpSerializer.h>  // HttpSerializer::serialize
#include <FalconHTTP/HTTP/HttpStatus.h>      // HttpStatus
#include <FalconHTTP/Core/Connection.h>      // Connection
// clang-format on

// clang-format off
#include <stdexcept> // std::exception
#include <charconv>  // std::from_chars
#include <cctype>    // std::tolower
// clang-format on

namespace FalconHTTP::Core {

namespace {
// Case-insensitive header-name comparison for the raw pre-parse scan
// below (HTTP header names are case-insensitive per RFC 7230 §3.2).
bool equalsIgnoreCase(std::string_view a, std::string_view b) noexcept {
    if (a.size() != b.size())
        return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) !=
            std::tolower(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}
} // namespace

// ============================================================
//  Section 1 — Constructor
// ============================================================

Server::Server(Routing::Router& router, std::size_t threadCount) noexcept
    : router_(&router), pool_(threadCount) {}

Server::Server(Routing::Router& router, const Config::ServerConfig& config) noexcept
    : router_(&router), pool_(config.threadCount), configuredPort_(config.port),
      maxHeaderSize_(config.maxHeaderSize), maxBodySize_(config.maxBodySize) {}

// ============================================================
//  Section 2 — Middleware Registration
// ============================================================

void Server::use(Middleware::MiddlewareFn middleware) {
    middleware_.push_back(std::move(middleware));
}

// ============================================================
//  Section 3 — Lifecycle
// ============================================================

bool Server::start(uint16_t port) {
    configuredPort_ = port;
    return listener_.start(port);
}

bool Server::start() {
    return listener_.start(configuredPort_);
}

void Server::run() {
    if (router_ == nullptr || !listener_.isListening()) {
        return;
    }

    running_.store(true, std::memory_order_release);

    while (running_.load(std::memory_order_acquire)) {
        Socket clientSocket = listener_.accept();

        if (!clientSocket.isValid())
            continue;

        (void)clientSocket.setNoDelay(true);

        Connection connection(std::move(clientSocket));

        pool_.detach(
            [this, conn = std::move(connection)]() mutable { handleConnection(std::move(conn)); });
    }
}

void Server::stop() noexcept {
    running_.store(false, std::memory_order_release);
    listener_.stop();
}

bool Server::isRunning() const noexcept {
    return running_.load(std::memory_order_acquire);
}

// ============================================================
//  Section 4 — Private Helpers
// ============================================================

void Server::handleConnection(Connection connection) {
    try {
        std::string raw = connection.receiveAvailable(DefaultReadChunk);

        if (raw.empty()) {
            connection.close();
            return;
        }

        std::size_t headerEnd = raw.find("\r\n\r\n");

        while (headerEnd == std::string::npos) {
            if (raw.size() > maxHeaderSize_) {
                HTTP::HttpResponse errorResponse(HTTP::HttpStatus::RequestHeaderFieldsTooLarge);
                errorResponse.setBody("Request header too large");
                std::string raw_response = HTTP::HttpSerializer::serialize(errorResponse);
                (void)connection.sendAll(raw_response.data(), raw_response.size());
                connection.close();
                return;
            }

            std::string more = connection.receiveAvailable(DefaultReadChunk);
            if (more.empty()) {
                connection.close();
                return;
            }
            raw += more;
            headerEnd = raw.find("\r\n\r\n");
        }

        std::size_t contentLength = 0;
        std::size_t headerSearchPos = 0;

        while (true) {
            std::size_t lineStart = headerSearchPos;
            std::size_t lineEnd = raw.find("\r\n", lineStart);
            if (lineEnd == std::string::npos || lineEnd > headerEnd)
                break;

            std::string_view line(raw.data() + lineStart, lineEnd - lineStart);
            std::size_t colon = line.find(':');

            if (colon != std::string_view::npos) {
                std::string_view name = line.substr(0, colon);
                if (equalsIgnoreCase(name, "Content-Length")) {
                    std::string_view value = line.substr(colon + 1);
                    while (!value.empty() && value.front() == ' ') {
                        value.remove_prefix(1);
                    }
                    auto [ptr, ec] =
                        std::from_chars(value.data(), value.data() + value.size(), contentLength);
                    if (ec != std::errc() || ptr != value.data() + value.size()) {
                        HTTP::HttpResponse errorResponse(HTTP::HttpStatus::BadRequest);
                        errorResponse.setBody("Malformed Content-Length header");
                        std::string raw_response = HTTP::HttpSerializer::serialize(errorResponse);
                        (void)connection.sendAll(raw_response.data(), raw_response.size());
                        connection.close();
                        return;
                    }
                }
            }

            headerSearchPos = lineEnd + 2;
        }

        if (contentLength > maxBodySize_) {
            HTTP::HttpResponse errorResponse(HTTP::HttpStatus::PayloadTooLarge);
            errorResponse.setBody("Request body too large");
            std::string raw_response = HTTP::HttpSerializer::serialize(errorResponse);
            (void)connection.sendAll(raw_response.data(), raw_response.size());
            connection.close();
            return;
        }

        std::size_t bodyStart = headerEnd + 4;
        std::size_t bodyBytesSoFar = raw.size() - bodyStart;

        while (bodyBytesSoFar < contentLength) {
            std::string more = connection.receiveAvailable(DefaultReadChunk);
            if (more.empty()) {
                connection.close();
                return;
            }
            raw += more;
            bodyBytesSoFar += more.size();
        }

        HTTP::HttpRequest request = HTTP::HttpParser::parse(raw);
        HTTP::HttpResponse response;

        runChain(0, request, response);

        std::string raw_response = HTTP::HttpSerializer::serialize(response);
        (void)connection.sendAll(raw_response.data(), raw_response.size());
    } catch (const std::exception&) {
        HTTP::HttpResponse errorResponse(HTTP::HttpStatus::InternalServerError);
        errorResponse.setBody("Internal Server Error");

        std::string raw_response = HTTP::HttpSerializer::serialize(errorResponse);
        (void)connection.sendAll(raw_response.data(), raw_response.size());
    }

    connection.close();
}

void Server::runChain(std::size_t index, HTTP::HttpRequest& request,
                      HTTP::HttpResponse& response) const {
    if (index >= middleware_.size()) {
        Routing::DispatchResult result = router_->dispatch(request, response);

        if (result == Routing::DispatchResult::NotFound) {
            response.setStatus(HTTP::HttpStatus::NotFound);
            response.setBody("Not Found");
        } else if (result == Routing::DispatchResult::MethodNotAllowed) {
            response.setStatus(HTTP::HttpStatus::MethodNotAllowed);
            response.setBody("Method Not Allowed");
        }
        return;
    }

    Middleware::NextHandler next(
        [this, index](HTTP::HttpRequest& request, HTTP::HttpResponse& response) {
            runChain(index + 1, request, response);
        });

    middleware_[index](request, response, next);
}

} // namespace FalconHTTP::Core