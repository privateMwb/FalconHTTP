#pragma once

#include "HashMap.h"

#include <string>
#include <string_view>

namespace FalconHTTP::Routing {

    class PathMatcher {
        public:

        // Core API
        [[nodiscard]] static bool match(
            std::string_view pattern,
            std::string_view path,
            HashMap<std::string, std::string>& params
        );
    };

} // namespace FalconHTTP::Routing