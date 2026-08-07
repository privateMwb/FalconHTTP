/**
 * @file StaticFileServer.cpp
 * @brief StaticFileServer implementation.
 *
 * Contains the implementation of StaticFileServer's construction, file
 * serving (cache lookup / disk read / cache population), and the private
 * path-safety and extension helpers.
 */

// ============================================================
// Implementation for FalconHTTP::FileServer::StaticFileServer.
// ============================================================
//
//  Sections:
//   1. Constructors
//   2. Core API
//   3. Private Helpers
//
// ============================================================

// clang-format off
#include <FalconHTTP/FileServer/StaticFileServer.h> // StaticFileServer (own header)
#include <FalconHTTP/HTTP/MimeTypes.h>               // mimeTypeFromExtension
#include <FalconHTTP/HTTP/HttpStatus.h>               // HttpStatus

#include <fstream> // std::ifstream
#include <sstream> // std::ostringstream
// clang-format on

namespace FalconHTTP::FileServer {

// ============================================================
//  Section 1 — Constructors
// ============================================================

StaticFileServer::StaticFileServer(std::string rootDirectory, std::size_t cacheCapacity)
    : rootDirectory_(std::filesystem::weakly_canonical(rootDirectory)), cache_(cacheCapacity) {}

// ============================================================
//  Section 2 — Core API
// ============================================================

bool StaticFileServer::serve(const std::string& requestPath, HTTP::HttpResponse& response) {
    FileCache::Entry cached;
    if (cache_.get(requestPath, cached)) {
        response.setStatus(HTTP::HttpStatus::Ok);
        response.setHeader("Content-Type", cached.contentType);
        response.setBody(cached.content);
        return true;
    }

    std::filesystem::path relative(requestPath.empty() ? "/" : requestPath);
    std::filesystem::path fullPath = rootDirectory_ / relative.relative_path();

    if (!isPathSafe(fullPath))
        return false;

    if (!std::filesystem::exists(fullPath) || !std::filesystem::is_regular_file(fullPath))
        return false;

    std::ifstream file(fullPath, std::ios::binary);
    if (!file.is_open())
        return false;

    std::ostringstream contents;
    contents << file.rdbuf();

    std::string contentType(HTTP::mimeTypeFromExtension(extensionOf(fullPath)));

    FileCache::Entry entry{contents.str(), contentType};
    cache_.put(requestPath, entry);

    response.setStatus(HTTP::HttpStatus::Ok);
    response.setHeader("Content-Type",
                       std::string(HTTP::mimeTypeFromExtension(extensionOf(fullPath))));
    response.setBody(contents.str());

    return true;
}

// ============================================================
//  Section 3 — Private Helpers
// ============================================================

bool StaticFileServer::isPathSafe(const std::filesystem::path& resolvedPath) const noexcept {
    std::error_code errorCode;
    std::filesystem::path canonicalPath =
        std::filesystem::weakly_canonical(resolvedPath, errorCode);

    if (errorCode)
        return false;

    auto rootIt = rootDirectory_.begin();
    auto pathIt = canonicalPath.begin();

    for (; rootIt != rootDirectory_.end(); ++rootIt, ++pathIt) {
        if (pathIt == canonicalPath.end() || *rootIt != *pathIt)
            return false;
    }

    return true;
}

std::string StaticFileServer::extensionOf(const std::filesystem::path& path) const noexcept {
    return path.extension().string();
}

} // namespace FalconHTTP::FileServer