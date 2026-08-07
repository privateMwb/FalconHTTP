/**
 * @file            Logger.h
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

// Middleware that logs one line per request (method, path, status,
// duration) to stdout after the rest of the chain has run.

namespace FalconHTTP::Middleware {

/**
 * @class Logger
 * @brief Logs one line per request to stdout after the rest of the
 *        chain (including the route handler) has run.
 *
 * @details
 * Format: `[INFO] METHOD path status reason durationMs ms`. Calls
 * `next` first, so the logged status/duration reflect the final
 * response after all downstream middleware and the handler have run.
 * Writes are serialized with an internal mutex, since Server dispatches
 * requests onto a thread pool and concurrent unsynchronized writes to
 * `std::cout` would otherwise interleave/garble output across requests.
 */
class Logger {
  public:
    // Core API
    void operator()(HTTP::HttpRequest& request, HTTP::HttpResponse& response,
                    const NextHandler& next) const;
};

} // namespace FalconHTTP::Middleware