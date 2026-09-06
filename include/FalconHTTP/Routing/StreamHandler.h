/**
 * @file            StreamHandler.h
 *
 * @date            2026-9-2
 *
 * @version         1.0.0
 *
 * @copyright       Copyright (c) 2026 MWB
 *                  All rights reserved.
 *                  https://github.com/privateMwb/FalconHTTP
 *
 * @attention       This source is released under the MIT license
 *                  SPDX-License-Identifier: MIT
 *                  <http://opensource.org/licenses/MIT>
 */

#pragma once

// clang-format off
#include <FalconHTTP/HTTP/HttpRequest.h> // HttpRequest

#include <FunctionPro/Function.h> // Function
// clang-format on

// Function-type alias for a streaming route's handler callback, invoked
// by Server (not Router::dispatch() - see Router::matchStream()) once a
// Stream-kind route has matched and the connection has been upgraded to
// an SseConnection.

namespace FalconHTTP::Streaming {
// Forward declaration - SseConnection is defined by the Streaming
// module (see SseConnection.h); only a reference is needed here.
class SseConnection;
} // namespace FalconHTTP::Streaming

namespace FalconHTTP::Routing {

/**
 * @brief The callback invoked when a Stream-kind Route's pattern
 *        matches an incoming request.
 * @details Mirrors RouteHandler's shape, but is given an
 *          Streaming::SseConnection& instead of an HttpResponse& -
 *          unlike a normal route handler, a stream handler may write
 *          to its connection repeatedly over time (see
 *          SseConnection::send()) rather than populating a single
 *          response object. Takes the request by const reference for
 *          the same reason as RouteHandler.
 */
using StreamHandler =
    FunctionPro::Function<void(const HTTP::HttpRequest&, Streaming::SseConnection&)>;

} // namespace FalconHTTP::Routing
