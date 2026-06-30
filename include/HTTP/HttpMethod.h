#pragma once

#include <string_view>

namespace FalconHTTP::HTTP {

    // Methods
    enum class HttpMethod {
        Get,
        Post,
        Put,
        Delete,
        Patch,
        Head,
        Options,
        Unknown
    };

    // Conversion
    [[nodiscard]] HttpMethod methodFromString(std::string_view text) noexcept;
    [[nodiscard]] std::string_view methodToString(HttpMethod method) noexcept;

} // namespace FalconHTTP::HTTP