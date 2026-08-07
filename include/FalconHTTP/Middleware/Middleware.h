/**
 * @file            Middleware.h
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
#include <FalconHTTP/HTTP/HttpRequest.h>  // HttpRequest
#include <FalconHTTP/HTTP/HttpResponse.h> // HttpResponse

#include <FunctionPro/Function.h> // Function
// clang-format on

using namespace FunctionPro;

// Function-type aliases for FalconHTTP's onion-model middleware chain,
// plus the built-in Cors, Logger, and Recovery middleware implementations.

namespace FalconHTTP::Middleware {

// Alias

/**
 * @brief Invokes the rest of the middleware chain (and eventually the
 *        matched route handler).
 * @details Middleware calls `next(request, response)` to continue the
 *          chain; not calling it short-circuits (subsequent middleware
 *          and the route handler never run) - used e.g. by Recovery to
 *          stop propagation after catching an exception.
 */
using NextHandler = FunctionPro::Function<void(HTTP::HttpRequest&, HTTP::HttpResponse&)>;

/**
 * @brief A single middleware step in the chain.
 * @details Registered via `Server::use()` (in registration order,
 *          onion model: each middleware runs before and after calling
 *          @p next). See Cors, Logger, and Recovery for reference
 *          implementations.
 */
using MiddlewareFn =
    FunctionPro::Function<void(HTTP::HttpRequest&, HTTP::HttpResponse&, const NextHandler&)>;

} // namespace FalconHTTP::Middleware