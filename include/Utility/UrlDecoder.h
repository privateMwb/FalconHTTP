#pragma once

#include <string>
#include <string_view>

namespace FalconHTTP::Utility {

    class UrlDecoder {
        public:

        // Core API
        [[nodiscard]] static std::string decode(std::string_view encoded);

        private:

        // Private Helpers
        [[nodiscard]] static int hexDigitToValue(char c) noexcept;
    };

} // namespace FalconHTTP::Utility 