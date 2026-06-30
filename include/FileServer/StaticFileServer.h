#pragma once

#include "HTTP/HttpResponse.h"
#include "FileCache.h"

#include <filesystem>
#include <string>

namespace FalconHTTP::FileServer {
    class StaticFileServer {
        private:

        // Storage
        std::filesystem::path  rootDirectory_;
        FileCache              cache_;

        public:
        
        // Constructors
        StaticFileServer() noexcept = default;
        explicit StaticFileServer(std::string rootDirectory, std:: size_t cacheCapacity);
        
        // Core API
        [[nodiscard]] bool serve(const std::string& requestPath, HTTP::HttpResponse& response);

        private:
        
        // Private Helpers
        [[nodiscard]] bool isPathSafe(const std::filesystem::path& resolvedPath) const noexcept;
        [[nodiscard]] std::string extensionOf(const std::filesystem::path& path) const noexcept;
    };

} // namespace FalconHTTP::FileServer