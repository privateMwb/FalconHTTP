// PathMatcher::match() test suite.
//
// Coverage:
// - Literal segments must match exactly
// - :name segments bind the corresponding path segment into params
// - Multiple :name segments in one pattern all bind correctly
// - Mismatched segment count (extra or missing) fails the match
// - Trailing slash is significant ("/users" != "/users/")
// - params may be partially populated on a false return (not rolled back)

#include <support/framework.h>

using namespace FalconHTTP::Routing;

// Verifies a pattern with no params matches only the identical path.
static void matches_literal_pattern() {
    HashMap<std::string, std::string> params;
    CHK(PathMatcher::match("/health", "/health", params));
    CHK(!PathMatcher::match("/health", "/healthz", params));
}

// Verifies a single :name segment binds the matching path segment.
static void binds_single_param() {
    HashMap<std::string, std::string> params;
    CHK(PathMatcher::match("/users/:id", "/users/42", params));
    CHK(params.contains("id"));
    CHK(params.at("id") == "42");
}

// Verifies multiple :name segments in one pattern all bind correctly.
static void binds_multiple_params() {
    HashMap<std::string, std::string> params;
    CHK(PathMatcher::match("/users/:id/posts/:postId", "/users/7/posts/99", params));
    CHK(params.at("id") == "7");
    CHK(params.at("postId") == "99");
}

// Verifies a path with a different number of segments than the
// pattern does not match.
static void rejects_segment_count_mismatch() {
    HashMap<std::string, std::string> params;
    CHK(!PathMatcher::match("/users/:id", "/users", params));
    CHK(!PathMatcher::match("/users/:id", "/users/42/extra", params));
}

// Verifies a trailing slash on the path makes an otherwise-identical
// pattern fail to match.
static void trailing_slash_is_significant() {
    HashMap<std::string, std::string> params;
    CHK(!PathMatcher::match("/users", "/users/", params));
}

// Executes all PathMatcher::match() test cases.
static void run_tests() {
    RUN(matches_literal_pattern);
    RUN(binds_single_param);
    RUN(binds_multiple_params);
    RUN(rejects_segment_count_mismatch);
    RUN(trailing_slash_is_significant);
}

REGISTER_TEST_SUITE();
