#pragma once

#include "LRUCache.h"

#include <string>
#include <chrono>

namespace FalconHTTP::FileServer {
    
    class FileCache {
        public:

        // Identity 
        struct Entry {
            std::string content;
            std::string contentType;
        };

        private: 

        // Storage
        LRUCache<std::string, Entry> cache_;

        public:

        // Constructors
        explicit FileCache(std::size_t capacity);

        // Core API
        [[nodiscard]] bool get(const std::string& path, Entry& outEntry);
        void put(const std::string& path, Entry entry);
        void invalidate(const std::string& path);
    };

} // namespace FalconHTTP::FileServer