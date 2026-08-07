/**
 * @file            MimeTypes.h
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

// File-extension to MIME-type lookup for static file serving.

namespace FalconHTTP::HTTP {
// Lookup

/**
 * @brief Maps a file extension to a MIME type.
 * @param extension The extension including its leading dot (e.g.
 *        ".html"), as returned by `std::filesystem::path::extension()`.
 *        Matching is exact-case; ".HTML" does not match ".html" and
 *        falls through to the default.
 * @return The corresponding MIME type for a small built-in set of
 *         common extensions, or "application/octet-stream" for any
 *         extension not in that set (including an empty extension).
 */
[[nodiscard]] std::string_view mimeTypeFromExtension(std::string_view extension) noexcept;

} // namespace FalconHTTP::HTTP