/**
 * @file            FalconHTTP.h
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
#include <FalconHTTP/Core/Server.h>                 // FalconHTTP::Core
#include <FalconHTTP/Core/Listener.h>
#include <FalconHTTP/Core/Server.h>
#include <FalconHTTP/Core/Socket.h>
#include <FalconHTTP/HTTP/HttpRequest.h>            // FalconHTTP::HTTP
#include <FalconHTTP/HTTP/HttpMethod.h>
#include <FalconHTTP/HTTP/HttpParser.h>
#include <FalconHTTP/HTTP/HttpResponse.h>
#include <FalconHTTP/HTTP/HttpSerializer.h>
#include <FalconHTTP/HTTP/HttpStatus.h>
#include <FalconHTTP/HTTP/MimeTypes.h>
#include <FalconHTTP/FileServer/FileCache.h>
#include <FalconHTTP/Middleware/Middleware.h>       // FalconHTTP::Middleware
#include <FalconHTTP/Middleware/Cors.h>
#include <FalconHTTP/Middleware/Logger.h>
#include <FalconHTTP/Middleware/Recovery.h>
#include <FalconHTTP/Routing/PathMatcher.h>
#include <FalconHTTP/Routing/Route.h>
#include <FalconHTTP/Routing/RouteHandler.h>
#include <FalconHTTP/Routing/Router.h>
#include <FalconHTTP/FileServer/StaticFileServer.h> // FalconHTTP::FileServer
#include <FalconHTTP/Utility/UrlDecoder.h>          // FalconHTTP::Utility
#include <FalconHTTP/Config/ServerConfig.h>         // FalconHTTP::Config
// clang-format on

// Single umbrella header pulling in every FalconHTTP sub-namespace and
// declaring the one rain reopen below. Users who only need one piece
// (e.g. just HttpRequest.h) can keep including that header directly and
// skip this one; this header exists for "give me everything" convenience
// and for the rain alias, which is deliberately declared in exactly one
// place rather than once per sub-namespace.

/**
 * @brief Umbrella alias so this library's types are reachable under
 *        `rain::`, alongside every other project library, while the
 *        true namespace (and all internal diagnostics) remains
 *        `FalconHTTP`.
 *
 * @details
 * `using namespace FalconHTTP;` only pulls in names declared directly in
 * `FalconHTTP` - since every actual type lives in a sub-namespace
 * (`FalconHTTP::Core`, `FalconHTTP::HTTP`, `FalconHTTP::Routing`,
 * `FalconHTTP::Middleware`, `FalconHTTP::FileServer`,
 * `FalconHTTP::Utility`, `FalconHTTP::Config`), this does NOT produce
 * flat names like `rain::Socket` or `rain::HttpRequest`. What it gives
 * you is the sub-namespaces themselves, reachable one level under `rain`
 * instead of under `FalconHTTP`:
 * `rain::Core::Socket`, `rain::HTTP::HttpRequest`, `rain::Routing::Router`,
 * `rain::Middleware::Cors`, `rain::FileServer::StaticFileServer`,
 * `rain::Utility::UrlDecoder`, `rain::Config::ServerConfig`.
 *
 * Declared here only, in this umbrella header - no other FalconHTTP
 * header reopens `rain`.
 */
namespace rain {
using namespace FalconHTTP;
}
