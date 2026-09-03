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
#include <FalconHTTP/Core/Server.h>              // Server (own header)
#include <FalconHTTP/HTTP/HttpParser.h>          // HttpParser::parse
#include <FalconHTTP/HTTP/HttpSerializer.h>      // HttpSerializer::serialize
#include <FalconHTTP/HTTP/HttpStatus.h>          // HttpStatus
#include <FalconHTTP/Core/Connection.h>          // Connection
#include <FalconHTTP/Streaming/SseConnection.h>  // SseConnection
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

namespace {
// Shared by both constructors: ServerConfig::maxStreamingConnections's
// "0 means auto" convention resolves to half of the pool (minimum 1).
std::size_t resolveMaxStreamingConnections(std::size_t configured, std::size_t threadCount) {
    if (configured != 0) {
        return configured;
    }
    return threadCount / 2 == 0 ? 1 : threadCount / 2;
}
} // namespace

Server::Server(Routing::Router& router, std::size_t threadCount) noexcept
    : router_(&router), pool_(threadCount),
      maxStreamingConnections_(resolveMaxStreamingConnections(0, threadCount)) {}

Server::Server(Routing::Router& router, const Config::ServerConfig& config) noexcept
    : router_(&router), pool_(config.threadCount), configuredPort_(config.port),
      maxHeaderSize_(config.maxHeaderSize), maxBodySize_(config.maxBodySize),
      maxStreamingConnections_(
          resolveMaxStreamingConnections(config.maxStreamingConnections, config.threadCount)) {}

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

    running_ = true;

    while (running_) {
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
    running_ = false;
    listener_.stop();
}

bool Server::isRunning() const noexcept {
    return running_;
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

        if (runChain(0, request, response, connection)) {
            // connection was moved into an SseConnection and handed
            // to a StreamHandler by runChain() - already run to
            // completion (handler returned or the peer disconnected)
            // and closed by SseConnection's destructor. Nothing left
            // to write or close here.
            return;
        }

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

bool Server::runChain(std::size_t index, HTTP::HttpRequest& request,
                      HTTP::HttpResponse& response, Connection& connection) const {
    if (index >= middleware_.size()) {
        if (const Routing::StreamHandler* streamHandler = router_->matchStream(request)) {
            if (activeStreams_.fetch_add(1, std::memory_order_relaxed) >=
                maxStreamingConnections_) {
                activeStreams_.fetch_sub(1, std::memory_order_relaxed);
                response.setStatus(HTTP::HttpStatus::ServiceUnavailable);
                response.setBody("Too many concurrent streaming connections");
                return false;
            }

            struct ActiveStreamGuard {
                std::atomic<std::size_t>& count;
                ~ActiveStreamGuard() { count.fetch_sub(1, std::memory_order_relaxed); }
            } guard{activeStreams_};

            Streaming::SseConnection sse(std::move(connection), response.headers());
            (*streamHandler)(request, sse);
            return true;
        }

        Routing::DispatchResult result = router_->dispatch(request, response);

        if (result == Routing::DispatchResult::NotFound) {
            response.setStatus(HTTP::HttpStatus::NotFound);
            response.setBody("Not Found");
        } else if (result == Routing::DispatchResult::MethodNotAllowed) {
            response.setStatus(HTTP::HttpStatus::MethodNotAllowed);
            response.setBody("Method Not Allowed");
        }
        return false;
    }

    bool streamed = false;

    Middleware::NextHandler next(
        [this, index, &connection, &streamed](HTTP::HttpRequest& request,
                                              HTTP::HttpResponse& response) {
            streamed = runChain(index + 1, request, response, connection);
        });

    middleware_[index](request, response, next);

    return streamed;
}

} // namespace FalconHTTP::Core