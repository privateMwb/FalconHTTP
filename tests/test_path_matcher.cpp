// Path Matcher Test Suite
// Validates route matching logic including literals, parameters, and edge cases.
//
// Covers:
// - literal route matching
// - root path handling
// - multi-segment literal matching
// - single and multiple path parameters
// - mismatched routes
// - segment count validation

#include "Routing/PathMatcher.h"
#include "HashMap.h"
#include "test_helper.h"

using namespace FalconHTTP::Routing;

// Shared parameter store used across tests
// NOTE: cleared in tests that rely on isolation
static HashMap<std::string, std::string> params(16);

// Exact match test
// Ensures identical root paths match successfully
static void exact_match() {
    CHK(PathMatcher::match("/", "/", params) == true);
}

// Multi-segment literal match test
// Ensures identical static multi-segment routes match
static void multi_segment() {
    CHK(PathMatcher::match("/users", "/users", params) == true);
}

// Mismatch test
// Ensures completely different routes do not match
static void mismatch() {
    CHK(PathMatcher::match("/users", "/posts", params) == false);
}

// Single parameter extraction test
// Ensures route parameters are captured correctly
static void single_param() {
    params.clear();

    CHK(PathMatcher::match("/users/:id", "/users/42", params) == true);
    CHK(params.contains("id") == true);
    CHK(params.at("id") == "42");
}

// Parameter mismatch test
// Ensures parameterized route does not match incorrect base path
static void param_with_mismatch() {
    params.clear();
    CHK(PathMatcher::match("/users/:id", "/posts/42", params) == false);
}

// Pattern longer than path test
// Ensures missing segments cause mismatch
static void pattern_longer() {
    CHK(PathMatcher::match("/users/:id", "/users", params) == false);
}

// Path longer than pattern test
// Ensures extra segments in path cause mismatch
static void path_longer() {
    CHK(PathMatcher::match("/users", "/users/42", params) == false);
}

// Multiple parameter extraction test
// Ensures multiple route parameters are correctly parsed and stored
static void multi_param() {
    params.clear();

    CHK(PathMatcher::match("/users/:userId/posts/:postId",
        "/users/7/posts/99",
        params) == true);

    CHK(params.at("userId") == "7");
    CHK(params.at("postId") == "99");
}

// Entry Point
// Runs all PathMatcher test cases
void run_path_matcher_tests() {
    setTitle("PathMatcher Tests");

    RUN(exact_match);
    RUN(multi_segment);
    RUN(mismatch);
    RUN(single_param);
    RUN(param_with_mismatch);
    RUN(pattern_longer);
    RUN(path_longer);
    RUN(multi_param);

    std::cout<< "\n";
}