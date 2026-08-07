/**
 * @file            PathMatcher.h
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
#include <HashMapPro/HashMap.h> // HashMap

#include <string>      // std::string
#include <string_view> // std::string_view
// clang-format on

using namespace HashMapPro;

// Segment-by-segment matcher for route patterns like "/users/:id",
// extracting :param path segments. No wildcard/catch-all support.

namespace FalconHTTP::Routing {

/**
 * @class PathMatcher
 * @brief Segment-by-segment matcher for route patterns like
 *        "/users/:id/posts/:postId".
 *
 * @details
 * Matching is purely segment-based (split on `/`); there is no
 * wildcard or catch-all segment support (e.g. no `*`), and no regex.
 * A pattern segment of the form `:name` matches any single path
 * segment and binds it into @p params; any other segment must match
 * the path segment exactly (case-sensitive).
 *
 * @note Trailing slashes are significant: pattern "/users" does NOT
 *       match path "/users/" (the trailing slash leaves a non-empty
 *       remainder on the path side after the pattern is exhausted).
 */
class PathMatcher {
  public:
    // Core API

    /**
     * @brief Attempts to match @p path against @p pattern.
     * @param pattern Route pattern, e.g. "/users/:id".
     * @param path Request path to match, e.g. "/users/42".
     * @param[out] params Populated with one entry per `:name` segment
     *        matched. @note On a `false` return, @p params may still
     *        contain entries from segments matched before the
     *        mismatching segment was reached - it is not rolled back.
     *        Callers iterating multiple candidate routes (see
     *        Router::dispatch()) should clear @p params before each
     *        match() call.
     * @return true if every segment of @p pattern matched a
     *         corresponding segment of @p path with nothing left over
     *         on either side; false otherwise.
     */
    [[nodiscard]] static bool match(std::string_view pattern, std::string_view path,
                                    HashMap<std::string, std::string>& params);
};

} // namespace FalconHTTP::Routing