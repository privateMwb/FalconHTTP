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

#include <support/framework.h>

using namespace FalconHTTP::Utility;

// Verifies %XX sequences decode to their corresponding byte.
static void decodes_percent_encoded_bytes() {
    CHK(UrlDecoder::decode("%41") == "A");
    CHK(UrlDecoder::decode("%2F") == "/");
    CHK(UrlDecoder::decode("hello%20world") == "hello world");
}

// Verifies '+' decodes to a literal space.
static void decodes_plus_as_space() {
    CHK(UrlDecoder::decode("a+b") == "a b");
    CHK(UrlDecoder::decode("+++") == "   ");
}

// Verifies characters needing no decoding pass through unchanged.
static void passes_through_ordinary_text() {
    CHK(UrlDecoder::decode("abcXYZ123") == "abcXYZ123");
    CHK(UrlDecoder::decode("") == "");
}

// Verifies a '%' not followed by two valid hex digits is emitted
// literally rather than dropped or treated as an error.
static void passes_through_invalid_percent_sequence() {
    CHK(UrlDecoder::decode("100% sure") == "100% sure");
    CHK(UrlDecoder::decode("%zz") == "%zz");
    CHK(UrlDecoder::decode("%4") == "%4");
    CHK(UrlDecoder::decode("%") == "%");
}

// Verifies both uppercase and lowercase hex digits are accepted, and
// can be mixed within the same sequence.
static void accepts_uppercase_and_lowercase_hex() {
    CHK(UrlDecoder::decode("%2f") == "/");
    CHK(UrlDecoder::decode("%2F") == "/");
    CHK(UrlDecoder::decode("%2f") == UrlDecoder::decode("%2F"));
    CHK(UrlDecoder::decode("%aF") == "\xaf");
}

// Executes all UrlDecoder::decode() test cases.
static void run_tests() {
    RUN(decodes_percent_encoded_bytes);
    RUN(decodes_plus_as_space);
    RUN(passes_through_ordinary_text);
    RUN(passes_through_invalid_percent_sequence);
    RUN(accepts_uppercase_and_lowercase_hex);
}

REGISTER_TEST_SUITE();
