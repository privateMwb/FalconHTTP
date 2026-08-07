/**
 * @file            FileCache.h
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
#include <CachePro/LRUCache.h> // LRUCache

#include <mutex>               // std::mutex
#include <string>              // std::string
#include <chrono>              // (currently unused - no TTL/mtime logic references it yet)
// clang-format on

using namespace CachePro;

// In-memory LRU cache of served file contents, keyed by request path.
// Thin wrapper over CachePro::LRUCache; no entry ever expires or is
// invalidated automatically (no TTL, no mtime/ETag tracking).

namespace FalconHTTP::FileServer {

/**
 * @class FileCache
 * @brief Thin wrapper over `CachePro::LRUCache` storing file bytes and
 *        MIME type per path.
 *
 * @note No entry ever expires or is invalidated automatically - there
 *       is no TTL and no mtime/ETag tracking. invalidate() must be
 *       called explicitly (nothing in FalconHTTP currently does this
 *       automatically when a file on disk changes).
 *
 * @note All public operations are thread-safe. Concurrent callers are
 *       serialized internally using a mutex because the underlying
 *       `LRUCache` is not thread-safe.
 */
class FileCache {
  public:
    // Identity

    /// A cached file's content and the `Content-Type` it was served with.
    struct Entry {
        std::string content;
        std::string contentType;
    };

  private:
    // Storage

    /// Underlying LRU cache storing cached file entries.
    LRUCache<std::string, Entry> cache_;

    /// Serializes access to the underlying cache.
    mutable std::mutex mutex_;

  public:
    // Constructors

    /// @param capacity Maximum number of entries the LRU cache holds
    ///        before evicting the least-recently-used entry.
    explicit FileCache(std::size_t capacity);

    // Core API

    /**
     * @param path Cache key - the request path as passed by the caller
     *        (see StaticFileServer::serve()'s note on key normalization).
     * @param[out] outEntry Set to the cached entry on a hit; left
     *        untouched on a miss.
     * @return true if @p path was found in the cache.
     *
     * @note Thread-safe.
     */
    [[nodiscard]] bool get(const std::string& path, Entry& outEntry);

    /**
     * @brief Inserts or updates the cached entry for @p path.
     *
     * May evict the least-recently-used entry if the cache is at
     * capacity.
     *
     * @note Thread-safe.
     */
    void put(const std::string& path, Entry entry);

    /**
     * @brief Removes @p path from the cache, if present.
     *
     * No-op if the entry does not exist.
     *
     * @note Thread-safe.
     */
    void invalidate(const std::string& path);
};

} // namespace FalconHTTP::FileServer