/**
 * @file            UrlDecoder.h
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
#include <string>      // std::string
#include <string_view> // std::string_view
// clang-format on

// Percent-decoding for URL-encoded (application/x-www-form-urlencoded)
// text, used for decoding query-string values.

namespace FalconHTTP::Utility {

/**
 * @class UrlDecoder
 * @brief Decodes `application/x-www-form-urlencoded`-style text.
 *
 * @note decode() also converts `+` to a literal space, which is
 *       correct for query strings and form bodies but NOT valid for
 *       raw path segments (where `+` is literal). FalconHTTP only
 *       applies this to query parameters (see HttpParser::parsePath);
 *       the request path itself is currently stored un-decoded.
 */
class UrlDecoder {
  public:
    // Core API

    /**
     * @brief Percent-decodes @p encoded.
     * @param encoded Text to decode (e.g. a query string value).
     * @return The decoded string. `%XX` sequences are converted to
     *         the corresponding byte; `+` is converted to a space.
     *         A `%` not followed by two valid hex digits (including
     *         at/near the end of the string) is passed through
     *         literally rather than treated as an error.
     */
    [[nodiscard]] static std::string decode(std::string_view encoded);

  private:
    // Private Helpers

    /// @return The value of hex digit @p c (0-15), or -1 if @p c is
    ///         not a valid hex digit.
    [[nodiscard]] static int hexDigitToValue(char c) noexcept;
};

} // namespace FalconHTTP::Utility
