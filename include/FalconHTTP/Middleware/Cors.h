/**
 * @file            Cors.h
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

#include <string> // std::string (allowedOrigin_)
// clang-format on

// Middleware that adds Access-Control-Allow-* headers to every response
// and short-circuits OPTIONS preflight requests with 204 No Content,
// without invoking the rest of the chain.

namespace FalconHTTP::Middleware {

/**
 * @class Cors
 * @brief Adds `Access-Control-Allow-*` headers to every response and
 *        short-circuits `OPTIONS` preflight requests.
 *
 * @details
 * Always sets `Access-Control-Allow-Origin`, `-Methods`
 * ("GET, POST, PUT, DELETE, PATCH, OPTIONS"), and `-Headers`
 * ("Content-Type, Authorization"). On an `OPTIONS` request, responds
 * with `204 No Content` and does NOT call `next` - the middleware
 * chain and route handler are skipped entirely for preflight requests.
 *
 * @note Advertises PATCH and OPTIONS as allowed methods, but Router
 *       currently has no registration method for either (only
 *       get/post/put/del) - a client attempting a real PATCH request
 *       past preflight will get 404/405, not a handler.
 */
class Cors {
  private:
    // Storage
    std::string allowedOrigin_;

  public:
    // Constructors

    /// Allows all origins ("*").
    Cors() noexcept;

    /// @param allowedOrigin Value for the `Access-Control-Allow-Origin`
    ///        header (e.g. a specific origin, or "*").
    explicit Cors(std::string allowedOrigin);

    // Core API
    void operator()(HTTP::HttpRequest& request, HTTP::HttpResponse& response,
                    const NextHandler& next) const;
};

} // namespace FalconHTTP::Middleware