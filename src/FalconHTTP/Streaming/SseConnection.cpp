/**
 * @file SseConnection.cpp
 * @brief SseConnection implementation.
 */

// clang-format off
#include <FalconHTTP/Streaming/SseConnection.h> // SseConnection (own header)
// clang-format on

namespace FalconHTTP::Streaming {

// Constructors
SseConnection::SseConnection(
    Core::Connection connection,
    const HashMapPro::HashMap<std::string, std::string>& extraHeaders) noexcept
    : connection_(std::move(connection)) {
    alive_ = sendPreamble(extraHeaders);
}

// Core API
bool SseConnection::send(std::string_view event, std::string_view data) noexcept {
    if (!alive_) {
        return false;
    }

    std::string frame;

    if (!event.empty()) {
        frame += "event: ";
        frame.append(event);
        frame += "\n";
    }

    std::size_t start = 0;
    while (true) {
        std::size_t newline = data.find('\n', start);
        std::string_view line = (newline == std::string_view::npos)
                                    ? data.substr(start)
                                    : data.substr(start, newline - start);

        frame += "data: ";
        frame.append(line);
        frame += "\n";

        if (newline == std::string_view::npos) {
            break;
        }

        start = newline + 1;
    }

    frame += "\n";

    if (!connection_.sendAll(frame.data(), frame.size())) {
        alive_ = false;
        return false;
    }

    return true;
}

bool SseConnection::isConnected() const noexcept {
    return alive_;
}

void SseConnection::close() noexcept {
    connection_.close();
    alive_ = false;
}

// Private Helpers
bool SseConnection::sendPreamble(
    const HashMapPro::HashMap<std::string, std::string>& extraHeaders) noexcept {
    std::string preamble = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/event-stream\r\n"
                           "Cache-Control: no-cache\r\n"
                           "Connection: close\r\n";

    for (const auto& entry : extraHeaders) {
        preamble += entry.key;
        preamble += ": ";
        preamble += entry.value;
        preamble += "\r\n";
    }

    preamble += "\r\n";

    return connection_.sendAll(preamble.data(), preamble.size());
}

} // namespace FalconHTTP::Streaming
