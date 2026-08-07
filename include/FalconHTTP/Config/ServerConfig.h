/**
 * @file            ServerConfig.h
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
#include <cstdint> // uint16_t
#include <string>  // std::string
// clang-format on

// Aggregate of server configuration values, consumed by
// Server(Router&, const ServerConfig&) for port/threadCount/DoS caps.
// staticRoot/fileCacheCapacity/corsAllowedOrigin are not read by Server
// itself (it doesn't own a StaticFileServer or Cors instance) - pass
// them to those constructors directly, e.g.
// StaticFileServer(config.staticRoot, config.fileCacheCapacity) and
// Cors(config.corsAllowedOrigin), so the whole application shares one
// config object.

namespace FalconHTTP::Config {

/**
 * @class ServerConfig
 * @brief Plain-data configuration struct for a Server instance and
 *        the file-serving/CORS pieces an application typically wires
 *        up alongside it.
 *
 * @details
 * port, threadCount, maxHeaderSize, and maxBodySize are read by
 * Server(Routing::Router&, const ServerConfig&) (see Server.h).
 * staticRoot, fileCacheCapacity, and corsAllowedOrigin are NOT read
 * by Server - it doesn't own a StaticFileServer or Cors instance -
 * they exist here so an application can construct those pieces from
 * the same shared config: `StaticFileServer(config.staticRoot,
 * config.fileCacheCapacity)`, `Cors(config.corsAllowedOrigin)`.
 */
class ServerConfig {
  public:
    // Identity

    /// Port passed to Listener::start() by Server::start() when
    /// Server was constructed from this config.
    uint16_t port = 8080;

    /// Thread pool size used by Server(Router&, const ServerConfig&).
    std::size_t threadCount = 4;

    /// Root directory for an application-constructed StaticFileServer.
    std::string staticRoot = "./public";

    /// FileCache capacity for an application-constructed StaticFileServer.
    std::size_t fileCacheCapacity = 64;

    /// Allowed origin for an application-constructed Cors middleware.
    std::string corsAllowedOrigin = "*";

    /// Maximum request-line + header block size Server will accept
    /// before responding 431 Request Header Fields Too Large (see
    /// Server::MaxHeaderSize's prior hardcoded default, now here).
    std::size_t maxHeaderSize = 16 * 1024; // 16 KiB

    /// Maximum request body size Server will accept (per
    /// Content-Length) before responding 413 Payload Too Large.
    std::size_t maxBodySize = 10 * 1024 * 1024; // 10 MiB
};

} // namespace FalconHTTP::Config