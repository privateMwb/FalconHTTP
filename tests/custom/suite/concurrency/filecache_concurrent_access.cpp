// FileCache concurrent access test suite.
//
// Coverage:
// - Many threads put()-ing distinct keys concurrently all succeed
//   without lost or corrupted entries, when capacity is large enough
//   that none get evicted
// - Many threads concurrently get()/put()/invalidate() on the SAME
//   shared key repeatedly complete without crashing or producing a
//   torn (partially-written) Entry - values read back are always one
//   of the values actually written, never a mix of two

#include <support/framework.h>

// clang-format off
#include <thread> // std::thread
#include <vector> // std::vector
#include <string> // std::to_string
#include <atomic> // std::atomic
// clang-format on

using namespace FalconHTTP::FileServer;

// Verifies many threads writing distinct keys concurrently all land
// correctly, with capacity large enough to guarantee no eviction.
static void distinct_keys_survive_concurrent_writes() {
    constexpr int threadCount = 16;

    FileCache cache(/*capacity=*/threadCount + 1);
    std::vector<std::thread> threads;

    for (int t = 0; t < threadCount; ++t) {
        threads.emplace_back([&cache, t]() {
            FileCache::Entry entry;
            entry.content = "content-" + std::to_string(t);
            entry.contentType = "text/plain";
            cache.put("/file-" + std::to_string(t), entry);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    for (int t = 0; t < threadCount; ++t) {
        FileCache::Entry outEntry;
        CHK(cache.get("/file-" + std::to_string(t), outEntry));
        CHK(outEntry.content == "content-" + std::to_string(t));
    }
}

// Verifies concurrent get()/put()/invalidate() on the same shared key
// from multiple threads completes without crashing, and any
// successful read returns a value that was actually written by some
// thread - never a torn mix of two writes.
static void shared_key_survives_concurrent_mixed_access() {
    constexpr int threadCount = 16;
    constexpr int operationsPerThread = 50;
    const std::string sharedKey = "/shared";

    FileCache cache(/*capacity=*/4);
    std::vector<std::thread> threads;
    std::atomic<bool> sawTornValue{false};

    for (int t = 0; t < threadCount; ++t) {
        threads.emplace_back([&cache, &sawTornValue, &sharedKey, t]() {
            for (int i = 0; i < operationsPerThread; ++i) {
                FileCache::Entry entry;
                entry.content = "writer-" + std::to_string(t);
                entry.contentType = "text/plain";
                cache.put(sharedKey, entry);

                FileCache::Entry outEntry;
                if (cache.get(sharedKey, outEntry)) {
                    // A torn value would not match the "writer-N"
                    // pattern any single put() call could have written.
                    bool matchesKnownWriter = false;
                    for (int writer = 0; writer < threadCount; ++writer) {
                        if (outEntry.content == "writer-" + std::to_string(writer)) {
                            matchesKnownWriter = true;
                            break;
                        }
                    }
                    if (!matchesKnownWriter) {
                        sawTornValue = true;
                    }
                }

                if (i % 7 == 0) {
                    cache.invalidate(sharedKey);
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    CHK(!sawTornValue.load());
}

// Executes all FileCache concurrent access test cases.
static void run_tests() {
    RUN(distinct_keys_survive_concurrent_writes);
    RUN(shared_key_survives_concurrent_mixed_access);
}

REGISTER_TEST_SUITE();
