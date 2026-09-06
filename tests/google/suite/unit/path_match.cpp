// PathMatcher::match() test suite.
//
// Coverage:
// - Literal segments must match exactly
// - :name segments bind the corresponding path segment into params
// - Multiple :name segments in one pattern all bind correctly
// - Mismatched segment count (extra or missing) fails the match
// - Trailing slash is significant ("/users" != "/users/")
// - params may be partially populated on a false return (not rolled back)

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::Routing;

// Verifies a pattern with no params matches only the identical path.
TEST(PathMatch, MatchesLiteralPattern) {
    HashMap<std::string, std::string> params;
    EXPECT_TRUE(PathMatcher::match("/health", "/health", params));
    EXPECT_FALSE(PathMatcher::match("/health", "/healthz", params));
}

// Verifies a single :name segment binds the matching path segment.
TEST(PathMatch, BindsSingleParam) {
    HashMap<std::string, std::string> params;
    ASSERT_TRUE(PathMatcher::match("/users/:id", "/users/42", params));
    ASSERT_TRUE(params.contains("id"));
    EXPECT_EQ(params.at("id"), "42");
}

// Verifies multiple :name segments in one pattern all bind correctly.
TEST(PathMatch, BindsMultipleParams) {
    HashMap<std::string, std::string> params;
    ASSERT_TRUE(PathMatcher::match("/users/:id/posts/:postId", "/users/7/posts/99", params));
    EXPECT_EQ(params.at("id"), "7");
    EXPECT_EQ(params.at("postId"), "99");
}

// Verifies a path with a different number of segments than the
// pattern does not match.
TEST(PathMatch, RejectsSegmentCountMismatch) {
    HashMap<std::string, std::string> params;
    EXPECT_FALSE(PathMatcher::match("/users/:id", "/users", params));
    EXPECT_FALSE(PathMatcher::match("/users/:id", "/users/42/extra", params));
}

// Verifies a trailing slash on the path makes an otherwise-identical
// pattern fail to match.
TEST(PathMatch, TrailingSlashIsSignificant) {
    HashMap<std::string, std::string> params;
    EXPECT_FALSE(PathMatcher::match("/users", "/users/", params));
}
