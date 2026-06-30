#include "Utility/UrlDecoder.h"

namespace FalconHTTP::Utility {

    // Core API
    std::string UrlDecoder::decode(std::string_view encoded) {
        std::string result;
        result.reserve(encoded.size());

        for (std::size_t i = 0; i < encoded.size(); ++i) {
            char c = encoded[i];

            if (c == '+') {
                result += ' ';
                continue;
            }

            if (c == '%' && i + 2 < encoded.size()) {
                int high  = hexDigitToValue(encoded[i + 1]);
                int low   = hexDigitToValue(encoded[i + 2]);
                
                if (high >= 0 && low >= 0) {
                    result += static_cast<char>((high << 4) | low);
                    i += 2;
                    continue;
                }
            }

            result += c;
        }

        return result;
    }

    // Private Helpers
    int UrlDecoder::hexDigitToValue(char c) noexcept {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return (c - 'a') + 10;
        if (c >= 'A' && c <= 'F') return (c - 'A') + 10;
        return -1;
    }

} // namespace FalconHTTP::Utility  