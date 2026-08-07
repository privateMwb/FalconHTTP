/**
 * @file            Recovery.h
 *
 * @date            2026-5-8
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
#include <FalconHTTP/Middleware/Middleware.h> // NextHandler
// clang-format on

// Middleware that catches exceptions from downstream middleware/handlers
// and converts them into a 500 Internal Server Error response instead of
// letting them propagate up and crash the handling thread.

namespace FalconHTTP::Middleware {

/**
 * @class Recovery
 * @brief Wraps `next` in a try/catch so an exception thrown by any
 *        downstream middleware or route handler becomes a
 *        `500 Internal Server Error` response instead of propagating
 *        up and crashing the handling thread.
 *
 * @details
 * Catches `std::exception` (logs `e.what()`) and any other thrown
 * value via `catch (...)` (logs "unknown exception"), in both cases
 * logging to stderr and setting the response to 500 with a generic
 * body. Error lines are written under an internal mutex for the same
 * reason as Logger (concurrent requests on the thread pool).
 *
 * @note For Recovery to protect a route, it must be registered via
 *       `Server::use()` before (outside) any middleware/handler that
 *       might throw - middleware registered after an exception site
 *       in the chain will not be reached. See the onion-model
 *       ordering note in Middleware.h.
 */
class Recovery {
  public:
    // Core API
    void operator()(HTTP::HttpRequest& request, HTTP::HttpResponse& response,
                    const NextHandler& next) const;
};

} // namespace FalconHTTP::Middleware