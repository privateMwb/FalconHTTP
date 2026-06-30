#pragma once

#include "HttpRequest.h"

#include <string_view>

namespace FalconHTTP::HTTP {

    class HttpParser {
    public:

        // Core API
        [[nodiscard]] static HttpRequest parse(std::string_view raw);

    private:

        // Private Helpers
        [[nodiscard]] static std::string_view extractLine(std::string_view& remaining);
        static void parseRequestLine(std::string_view line, HttpRequest& request);
        static void parseHeaderLine(std::string_view line, HttpRequest& request);
        static void parsePath(std::string_view rawPath, HttpRequest& request);
    };

} // namespace FalconHTTP::HTTP