/**
 * @file            StaticFileServer.h
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
#include <FalconHTTP/HTTP/HttpResponse.h>    // HttpResponse
#include <FalconHTTP/FileServer/FileCache.h> // FileCache

#include <filesystem> // std::filesystem::path
#include <string>     // std::string
// clang-format on

// Resolves a request path to a file under a fixed root directory and
// serves its contents, with an in-memory LRU content cache (FileCache).

namespace FalconHTTP::FileServer {

/**
 * @class StaticFileServer
 * @brief Resolves a request path to a file under a fixed root
 *        directory and serves its contents.
 *
 * @details
 * Not thread-safe on its own beyond whatever thread-safety FileCache
 * provides internally - if used concurrently across the thread pool,
 * verify FileCache's guarantees.
 *
 * @note Known limitations:
 * - Every served file is read fully into memory (`std::ostringstream`
 *   over the whole file) with no size cap or streaming - very large
 *   files will spike per-request memory use.
 * - Once cached, an entry is never invalidated automatically (no
 *   mtime/ETag check) - if the file on disk changes, the stale cached
 *   copy keeps being served until evicted by the LRU or manually
 *   invalidated via the underlying FileCache.
 * - No conditional-GET support (`If-None-Match`, `If-Modified-Since`);
 *   every successful request re-sends the full body.
 * - The cache key is the raw @p requestPath as passed by the caller -
 *   if that path is not normalized/decoded consistently by the
 *   caller, logically-identical requests could miss the cache or
 *   collide.
 */
class StaticFileServer {
  private:
    // Storage
    std::filesystem::path rootDirectory_;
    FileCache cache_;

  public:
    // Constructors
    StaticFileServer() = delete;

    /**
     * @param rootDirectory Directory files are served from. Resolved
     *        via `std::filesystem::weakly_canonical` at construction
     *        time - all served paths are checked (see isPathSafe())
     *        to resolve within this directory.
     * @param cacheCapacity Maximum number of entries in the LRU
     *        content cache.
     */
    explicit StaticFileServer(std::string rootDirectory, std::size_t cacheCapacity = 256);

    // Core API

    /**
     * @brief Serves the file at @p requestPath, if it exists and
     *        resolves safely within the root directory.
     * @param requestPath Path relative to the root directory (e.g.
     *        "/index.html").
     * @param[out] response On success (true), populated with status
     *        200, `Content-Type` (from MimeTypes), and the file's
     *        contents as the body. Left untouched on failure (false) -
     *        caller is responsible for the 404 response.
     * @return true if the file was found and served (from cache or
     *         disk); false if it doesn't exist, isn't a regular file,
     *         couldn't be opened, or the resolved path fails the
     *         directory-traversal check.
     */
    [[nodiscard]] bool serve(const std::string& requestPath, HTTP::HttpResponse& response);

  private:
    // Private Helpers

    /// @return true if @p resolvedPath, once canonicalized, still
    ///         falls within rootDirectory_ (component-wise prefix
    ///         check) - the directory-traversal guard against `..`
    ///         segments and symlink escapes. Returns false if
    ///         canonicalization fails (e.g. broken symlink).
    [[nodiscard]] bool isPathSafe(const std::filesystem::path& resolvedPath) const noexcept;

    /// @return The file extension including its leading dot (e.g.
    ///         ".html"), or an empty string if none - passed directly
    ///         to mimeTypeFromExtension().
    [[nodiscard]] std::string extensionOf(const std::filesystem::path& path) const noexcept;
};

} // namespace FalconHTTP::FileServer