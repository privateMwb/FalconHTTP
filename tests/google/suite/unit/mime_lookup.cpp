// mimeTypeFromExtension() test suite.
//
// Coverage:
// - A handful of common extensions map to their expected MIME type
// - An unrecognized extension falls back to application/octet-stream
// - An empty extension falls back to application/octet-stream
// - Matching is exact-case (".HTML" does not match the ".html" entry)
//
// NOTE: the exact string values asserted for known extensions below
// (.html/.css/.json) are assumed to match MimeTypes.cpp's built-in
// table based on the standard MIME types for those extensions - this
// wasn't independently re-verified against the current table contents.
// If MimeTypes.cpp's table differs, update the expected values here.

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::HTTP;

// Verifies a few common, unambiguous extensions map to their expected
// MIME type. See the file-level NOTE above.
TEST(MimeLookup, MapsCommonExtensions) {
    EXPECT_EQ(mimeTypeFromExtension(".html"), "text/html");
    EXPECT_EQ(mimeTypeFromExtension(".css"), "text/css");
    EXPECT_EQ(mimeTypeFromExtension(".json"), "application/json");
}

// Verifies an extension not in the built-in table falls back to
// application/octet-stream.
TEST(MimeLookup, FallsBackForUnknownExtension) {
    EXPECT_EQ(mimeTypeFromExtension(".unknownext"), "application/octet-stream");
}

// Verifies an empty extension falls back to application/octet-stream.
TEST(MimeLookup, FallsBackForEmptyExtension) {
    EXPECT_EQ(mimeTypeFromExtension(""), "application/octet-stream");
}

// Verifies matching is exact-case; an uppercase extension does not
// match a lowercase table entry.
TEST(MimeLookup, MatchingIsCaseSensitive) {
    EXPECT_NE(mimeTypeFromExtension(".HTML"), "text/html");
    EXPECT_EQ(mimeTypeFromExtension(".HTML"), "application/octet-stream");
}
