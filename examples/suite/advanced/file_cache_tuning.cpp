// Tuning the static-file cache.
//
// Demonstrates:
// - Choosing FileCache's capacity via StaticFileServer's constructor
// - A subtlety this library has no protection against: once a file is
//   cached, editing it on disk does NOT invalidate the cached copy
// - FileCache's own get()/put()/invalidate() API for manual invalidation
// - Why that manual invalidation can't be reached through StaticFileServer:
//   its cache_ member is private with no accessor

#include <support/framework.h>

#include <filesystem>
#include <fstream>
#include <iostream>

using namespace FalconHTTP;

static void run_examples() {

    // cacheCapacity is fixed at construction - it bounds how many distinct
    // files' contents StaticFileServer will hold in memory at once before
    // evicting the least-recently-used entry.
    setTitle("Capacity Choice");

    std::filesystem::path root =
        std::filesystem::temp_directory_path() / "falconhttp_cache_example";
    std::filesystem::create_directories(root);
    std::ofstream(root / "notice.txt") << "original content";

    FileServer::StaticFileServer staticFiles(root.string(), /*cacheCapacity=*/2);

    std::cout << "cache capacity : 2 entries\n\n";

    // The first serve() reads from disk and populates the cache.
    setTitle("Caching, Then Editing the File");

    HTTP::HttpResponse first;
    (void)staticFiles.serve("/notice.txt", first);
    std::cout << "first body  : " << first.body() << "\n";

    // Editing the file on disk after it's cached does not invalidate the
    // cached copy - there's no mtime/ETag check anywhere in this path.
    std::ofstream(root / "notice.txt") << "updated content";

    HTTP::HttpResponse second;
    (void)staticFiles.serve("/notice.txt", second);
    std::cout << "second body : " << second.body() << " (stale - still the original)\n\n";

    // StaticFileServer doesn't expose its internal FileCache, so an
    // application that needs manual invalidation should own and share its
    // own FileCache instance directly rather than relying on the one
    // hidden inside StaticFileServer.
    setTitle("Manual Invalidation via a Shared FileCache");

    FileServer::FileCache cache(/*capacity=*/16);

    FileServer::FileCache::Entry entry{"cached bytes", "text/plain"};
    cache.put("/notice.txt", entry);

    FileServer::FileCache::Entry found;
    std::cout << "hit before invalidate : " << cache.get("/notice.txt", found) << "\n";

    cache.invalidate("/notice.txt");

    FileServer::FileCache::Entry afterInvalidate;
    std::cout << "hit after invalidate  : " << cache.get("/notice.txt", afterInvalidate) << "\n";

    std::filesystem::remove_all(root);
}

REGISTER_EXAMPLE_SUITE();
