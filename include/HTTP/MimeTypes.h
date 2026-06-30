#pragma once

#include <string_view>

namespace FalconHTTP::HTTP {

    // Lookup
    [[nodiscard]] std::string_view mimeTypeFromExtension(std::string_view extension) noexcept;

} // namespace FalconHTTP::HTTP