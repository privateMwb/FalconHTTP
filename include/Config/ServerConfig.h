#pragma once

#include <cstdint>
#include <string>

namespace FalconHTTP::Config {
    class ServerConfig {
        public:

        // Identity
        uint16_t     port               = 8080;
        std::size_t  threadCount        = 4;
        std::string  staticRoot         = "./public";
        std::size_t  fileCacheCapacity  = 64;
        std::string  corsAllowedOrigin  = "*";
    };

} // namespace FalconHTTP::Config