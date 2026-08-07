/**
 * @file FileCache.cpp
 * @brief FileCache implementation.
 */

// clang-format off
#include <FalconHTTP/FileServer/FileCache.h> // FileCache (own header)
// clang-format on

#include <mutex>

namespace FalconHTTP::FileServer {

// Constructors

FileCache::FileCache(std::size_t capacity) : cache_(capacity) {}

// Core API

bool FileCache::get(const std::string& path, Entry& outEntry) {
    std::lock_guard<std::mutex> lock(mutex_);

    Entry* found = cache_.get(path);

    if (found == nullptr) {
        return false;
    }

    outEntry = *found;
    return true;
}

void FileCache::put(const std::string& path, Entry entry) {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_.put(path, std::move(entry));
}

void FileCache::invalidate(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    (void)cache_.erase(path);
}

} // namespace FalconHTTP::FileServer