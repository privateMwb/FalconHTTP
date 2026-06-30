#include "FileServer/FileCache.h"

namespace FalconHTTP::FileServer {

    // Constructors
    FileCache::FileCache(std::size_t capacity) 
    : cache_(capacity) {}

    // Core API
    bool FileCache::get(const std::string&path, Entry& outEntry) {
        Entry* found = cache_.get(path);

        if (found == nullptr) return false;

        outEntry = *found;
        return true;
    }

    void FileCache::put(const std::string& path, Entry entry) {
        cache_.put(path, std::move(entry));
    }

    void FileCache::invalidate(const std::string& path) {
        (void)cache_.erase(path);
    }

} // namespace FalconHTTP::FileServer