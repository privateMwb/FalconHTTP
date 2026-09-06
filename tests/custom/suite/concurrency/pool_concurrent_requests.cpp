// Concurrent request dispatch test suite.
//
// Coverage:
// - Many "requests" (parsed HttpRequest/HttpResponse pairs) dispatched
//   concurrently against one shared Router instance each reach the
//   correct handler with no cross-request state leakage
//
// NOTE: this does not literally spin up a Server and drive it with
// concurrent client connections, because Socket has no client-side
// connect() anywhere in the public API - there is currently no way to
// originate an outbound TCP connection using FalconHTTP's own types,
// so a true multi-connection test against a live Listener isn't
// achievable without raw platform socket code outside the library.
// Instead, this suite targets the actual shared-state hazard Server's
// thread pool creates in production: many pooled threads calling
// Router::dispatch() concurrently on the one Router instance a Server
// holds. Each thread here uses its own local HttpRequest/HttpResponse
// (as Server::handleConnection() does per-connection), so this
// verifies Router's route table and PathMatcher's per-call params are
// safe to read concurrently - it does not exercise Listener::accept(),
// Connection I/O, or ThreadPoolPro's own scheduling.

#include <support/framework.h>

// clang-format off
#include <thread> // std::thread
#include <vector> // std::vector
#include <atomic> // std::atomic
#include <string> // std::to_string
// clang-format on

using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Routing;

// Verifies many threads dispatching distinct requests concurrently
// against one shared Router each reach the correct handler with the
// correct path param, and no thread observes another thread's data.
static void concurrent_dispatch_has_no_cross_request_leakage() {
    constexpr int threadCount = 16;
    constexpr int requestsPerThread = 50;

    Router router;
    std::atomic<int> handlerInvocations{0};

    router.get("/items/:id",
               [&handlerInvocations](const HttpRequest& request, HttpResponse& response) {
                   ++handlerInvocations;
                   // Echo the id straight back - if another thread's params were
                   // visible here instead, this would return the wrong value.
                   response.setStatus(HttpStatus::Ok);
                   response.setBody(request.pathParam("id"));
               });

    std::vector<std::thread> threads;
    std::atomic<bool> sawMismatch{false};

    for (int t = 0; t < threadCount; ++t) {
        threads.emplace_back([&router, &sawMismatch, t]() {
            for (int i = 0; i < requestsPerThread; ++i) {
                std::string id = std::to_string(t) + "-" + std::to_string(i);

                HttpRequest request;
                request.setMethod(HttpMethod::Get);
                request.setPath("/items/" + id);

                HttpResponse response;
                DispatchResult result = router.dispatch(request, response);

                if (result != DispatchResult::Matched || response.body() != id) {
                    sawMismatch = true;
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    CHK(!sawMismatch.load());
    CHK(handlerInvocations.load() == threadCount * requestsPerThread);
}

// Verifies concurrent dispatch against routes with different methods
// on overlapping paths resolves each request to the correct handler,
// even under contention (exercises the pathMatchedOtherMethod check
// in Router::dispatch() running concurrently across threads).
static void concurrent_dispatch_resolves_correct_method_under_contention() {
    Router router;
    std::atomic<int> getCount{0};
    std::atomic<int> postCount{0};

    router.get("/resource", [&getCount](const HttpRequest&, HttpResponse& response) {
        ++getCount;
        response.setStatus(HttpStatus::Ok);
    });
    router.post("/resource", [&postCount](const HttpRequest&, HttpResponse& response) {
        ++postCount;
        response.setStatus(HttpStatus::Created);
    });

    constexpr int iterationsPerThread = 100;
    std::vector<std::thread> threads;
    std::atomic<bool> sawWrongStatus{false};

    for (int t = 0; t < 8; ++t) {
        bool useGet = (t % 2 == 0);
        threads.emplace_back([&router, &sawWrongStatus, useGet]() {
            for (int i = 0; i < iterationsPerThread; ++i) {
                HttpRequest request;
                request.setMethod(useGet ? HttpMethod::Get : HttpMethod::Post);
                request.setPath("/resource");

                HttpResponse response;
                (void)router.dispatch(request, response);

                HttpStatus expected = useGet ? HttpStatus::Ok : HttpStatus::Created;
                if (response.status() != expected) {
                    sawWrongStatus = true;
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    CHK(!sawWrongStatus.load());
    CHK(getCount.load() == 4 * iterationsPerThread);
    CHK(postCount.load() == 4 * iterationsPerThread);
}

// Executes all concurrent dispatch test cases.
static void run_tests() {
    RUN(concurrent_dispatch_has_no_cross_request_leakage);
    RUN(concurrent_dispatch_resolves_correct_method_under_contention);
}

REGISTER_TEST_SUITE();
