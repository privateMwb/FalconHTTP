#include "Core/Server.h"
#include "HTTP/HttpParser.h"
#include "HTTP/HttpSerializer.h"
#include "HTTP/HttpStatus.h"
#include "Core/Connection.h"

#include <stdexcept>
#include <charconv>

namespace FalconHTTP::Core {

    // Constructor
    Server::Server(Routing::Router& router, std::size_t threadCount) noexcept
        : router_(&router)
        , pool_(threadCount) {
    }

    // Middleware Registration
    void Server::use(Middleware::MiddlewareFn middleware) {
        middleware_.push_back(std::move(middleware));
    }

    // Lifecycle
    bool Server::start(uint16_t port) {
        return listener_.start(port);
    }

    void Server::run() {
        if (router_ == nullptr || !listener_.isListening()) {
            return;
        }

        running_ = true;

        while (running_) {
            Socket clientSocket = listener_.accept();

            if (!clientSocket.isValid()) continue;

            (void)clientSocket.setNoDelay(true);

            Connection connection(std::move(clientSocket));

            pool_.execute([this, conn = std::move(connection)]() mutable {
                handleConnection(std::move(conn));
                });
        }
    }

    void Server::stop() noexcept {
        running_ = false;
        listener_.stop();
    }

    bool Server::isRunning() const noexcept {
        return running_;
    }

    // Private Helpers
    void Server::handleConnection(Connection connection) {
        try {
            std::string raw = connection.receiveAvailable(DefaultReadChunk);

            if (raw.empty()) {
                connection.close();
                return;
            }

            std::size_t headerEnd = raw.find("\r\n\r\n");

            while (headerEnd == std::string::npos) {
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
                if (lineEnd == std::string::npos || lineEnd > headerEnd) break;

                std::string_view line(raw.data() + lineStart, lineEnd - lineStart);
                std::size_t colon = line.find(':');

                if (colon != std::string_view::npos) {
                    std::string_view name = line.substr(0, colon);
                    if (name == "Content-Length") {
                        std::string_view value = line.substr(colon + 1);
                        while (!value.empty() && value.front() == ' ') {
                            value.remove_prefix(1);
                        }
                        std::from_chars(value.data(), value.data() + value.size(), contentLength);
                    }
                }

                headerSearchPos = lineEnd + 2;
            }

            std::size_t bodyStart = headerEnd = 4;
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
        }
        catch (const std::exception&) {
            HTTP::HttpResponse errorResponse(HTTP::HttpStatus::InternalServerError);
            errorResponse.setBody("Internal Server Error");

            std::string raw_response = HTTP::HttpSerializer::serialize(errorResponse);
            (void)connection.sendAll(raw_response.data(), raw_response.size());
        }

        connection.close();
    }

    void Server::runChain(std::size_t index, HTTP::HttpRequest& request, HTTP::HttpResponse& response) const {
        if (index >= middleware_.size()) {
            bool matched = router_->dispatch(request, response);

            if (!matched) {
                response.setStatus(HTTP::HttpStatus::NotFound);
                response.setBody("Not Found");
            }
            return;
        }

        Middleware::NextHandler next(
            [this, index](HTTP::HttpRequest& request, HTTP::HttpResponse& response) {
                runChain(index + 1, request, response);
            }
        );

        middleware_[index](request, response, next);
    }

} // namespace FalconHTTP::Core