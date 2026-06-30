#pragma once

#include "HttpResponse.h"

#include <string>

namespace FalconHTTP::HTTP {
    class HttpSerializer {
    public:

        // Core API
        [[nodiscard]] static std::string serialize(const HttpResponse& response);

    private:
        
        // Private Helpers
        static void appendStatusLine(std::string& output, const HttpResponse& response);
        static void appendHeaders(std::string& output, const HttpResponse& response);    
    };

} // namespace FalconHTTP::HTTP