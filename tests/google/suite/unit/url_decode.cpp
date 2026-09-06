// UrlDecoder::decode() test suite.
//
// Coverage:
// - Percent-encoded bytes are decoded (%XX -> byte)
// - '+' is decoded as a literal space
// - Ordinary characters pass through unchanged
// - An invalid or truncated %XX sequence is passed through literally
//   rather than treated as an error
// - Uppercase and lowercase hex digits are both accepted
// - Empty input decodes to an empty string

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::Utility;

// Verifies %XX sequences decode to their corresponding byte.
TEST(UrlDecode, DecodesPercentEncodedBytes) {
    EXPECT_EQ(UrlDecoder::decode("%41"), "A");
    EXPECT_EQ(UrlDecoder::decode("%2F"), "/");
    EXPECT_EQ(UrlDecoder::decode("hello%20world"), "hello world");
}

// Verifies '+' decodes to a literal space.
TEST(UrlDecode, DecodesPlusAsSpace) {
    EXPECT_EQ(UrlDecoder::decode("a+b"), "a b");
    EXPECT_EQ(UrlDecoder::decode("+++"), "   ");
}

// Verifies characters needing no decoding pass through unchanged.
TEST(UrlDecode, PassesThroughOrdinaryText) {
    EXPECT_EQ(UrlDecoder::decode("abcXYZ123"), "abcXYZ123");
    EXPECT_EQ(UrlDecoder::decode(""), "");
}

// Verifies a '%' not followed by two valid hex digits is emitted
// literally rather than dropped or treated as an error.
TEST(UrlDecode, PassesThroughInvalidPercentSequence) {
    EXPECT_EQ(UrlDecoder::decode("100% sure"), "100% sure");
    EXPECT_EQ(UrlDecoder::decode("%zz"), "%zz");
    EXPECT_EQ(UrlDecoder::decode("%4"), "%4");
    EXPECT_EQ(UrlDecoder::decode("%"), "%");
}

// Verifies both uppercase and lowercase hex digits are accepted, and
// can be mixed within the same sequence.
TEST(UrlDecode, AcceptsUppercaseAndLowercaseHex) {
    EXPECT_EQ(UrlDecoder::decode("%2f"), "/");
    EXPECT_EQ(UrlDecoder::decode("%2F"), "/");
    EXPECT_EQ(UrlDecoder::decode("%2f"), UrlDecoder::decode("%2F"));
    EXPECT_EQ(UrlDecoder::decode("%aF"), "\xaf");
}
