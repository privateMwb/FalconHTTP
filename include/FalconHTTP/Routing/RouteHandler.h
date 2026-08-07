/**
 * @file            RouteHandler.h
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

// Function-type alias for a route's handler callback, invoked by
// Router::dispatch() after the middleware chain has run.

namespace FalconHTTP::Routing {

// Alias

/**
 * @brief The callback invoked when a Route's method and pattern match
 *        an incoming request.
 * @details Takes the request by const reference (route handlers are
 *          not expected to mutate the request further - use
 *          middleware for that) and the response by mutable reference
 *          to populate the reply. Invoked by Router::dispatch() after
 *          the full middleware chain has run.
 */
using RouteHandler = FunctionPro::Function<void(const HTTP::HttpRequest&, HTTP::HttpResponse&)>;

} // namespace FalconHTTP::Routing
