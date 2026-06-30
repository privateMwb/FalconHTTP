// UrlDecoder Test Suite
// Validates URL decoding behavior including percent-decoding, plus-to-space conversion,
// case-insensitive hex handling, and malformed input resilience.
//
// Covers:
// - plain strings (no decoding needed)
// - '+' decoding to space
// - percent-encoded characters (%XX)
// - mixed case hex decoding
// - malformed percent sequences
// - real-world encoded query strings

#include "Utility/UrlDecoder.h"

#include "test_helper.h"

using namespace FalconHTTP::Utility;

// Plain string test
// Ensures strings without encoding remain unchanged
static void plain_test_unchanged() {
    CHK(UrlDecoder::decode("hello") == "hello");
}

// Plus-to-space decoding test
// Ensures '+' characters are converted to spaces
static void plus_as_space() {
    CHK(UrlDecoder::decode("hello+world") == "hello world");
}

// Percent-encoded space decoding test
// Ensures "%20" is correctly decoded into a space
static void percent_encoded_space() {
    CHK(UrlDecoder::decode("hello%20world") == "hello world");
}

// Percent-encoded symbol decoding test
// Ensures encoded special characters like %2B are decoded correctly
static void percent_encoded_plus_literal() {
    CHK(UrlDecoder::decode("C%2B%2B") == "C++");
}

// Hex case-insensitivity test
// Ensures both lowercase and uppercase hex values are decoded correctly
static void lowercase_and_uppercase_hex() {
    CHK(UrlDecoder::decode("%2b") == "+");
    CHK(UrlDecoder::decode("%2B") == "+");
    CHK(UrlDecoder::decode("%2b%2B") == "++");
}

// Malformed percent sequence test (end of string)
// Ensures incomplete percent encoding is left unchanged safely
static void malformed_percent_at_end() {
    CHK(UrlDecoder::decode("abc%") == "abc%");
    CHK(UrlDecoder::decode("abc%2") == "abc%2");
}

// Invalid hex digit handling test
// Ensures invalid percent sequences are not decoded and remain intact
static void malformed_invalid_hex_digits() {
    CHK(UrlDecoder::decode("100%GG") == "100%GG");
    CHK(UrlDecoder::decode("100%2G") == "100%2G");
}

// Real-world query decoding test
// Ensures full query strings with mixed encoding decode correctly
static void mixed_real_world_query() {
    CHK(UrlDecoder::decode("hello+world%21+how%27s+it+going%3F") == "hello world! how's it going?");
}

// Test runner for UrlDecoder
// Executes all URL decoding test cases
void run_url_decoder_tests() {
    setTitle("UrlDecoder Tests");

    RUN(plain_test_unchanged);
    RUN(plus_as_space);
    RUN(percent_encoded_space);
    RUN(percent_encoded_plus_literal);
    RUN(lowercase_and_uppercase_hex);
    RUN(malformed_percent_at_end);
    RUN(malformed_invalid_hex_digits);
    RUN(mixed_real_world_query);

    std::cout << "\n";
}