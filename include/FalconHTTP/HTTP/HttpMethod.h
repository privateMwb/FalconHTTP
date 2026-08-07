/**
 * @file            HttpMethod.h
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
#include <string_view> // std::string_view
// clang-format on

// HTTP request method enumeration and string conversion. Router currently
// only supports registering handlers for Get/Post/Put/Delete; Patch,
// Head, and Options are represented here but have no registration path yet.

namespace FalconHTTP::HTTP {

/**
 * @enum HttpMethod
 * @brief The set of HTTP methods FalconHTTP recognizes.
 * @note Router currently only supports registering handlers for Get,
 *       Post, Put, and Delete (see Router::get/post/put/del). Patch,
 *       Head, and Options are parsed and represented here, but there
 *       is no corresponding Router registration method yet.
 */
enum class HttpMethod {
    Get,
    Post,
    Put,
    Delete,
    Patch,
    Head,
    Options,
    /// Any method token not matching a known value above.
    Unknown
};

// Conversion

/**
 * @brief Parses an HTTP method token into an HttpMethod.
 * @param text The method token as it appears on the request line
 *        (e.g. "GET"). Matching is exact-case, uppercase only, per
 *        RFC 7230 (method tokens are case-sensitive) - "get" does
 *        not match HttpMethod::Get.
 * @return The matching HttpMethod, or HttpMethod::Unknown if @p text
 *         does not match a recognized method.
 */
[[nodiscard]] HttpMethod methodFromString(std::string_view text) noexcept;

/**
 * @brief Converts an HttpMethod back to its wire representation.
 * @return The uppercase method token (e.g. "GET"), or "UNKNOWN" for
 *         HttpMethod::Unknown or any unrecognized enum value.
 */
[[nodiscard]] std::string_view methodToString(HttpMethod method) noexcept;

} // namespace FalconHTTP::HTTP