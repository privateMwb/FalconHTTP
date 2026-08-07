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

#include <support/framework.h>

using namespace FalconHTTP::HTTP;

// Verifies a few common, unambiguous extensions map to their expected
// MIME type. See the file-level NOTE above.
static void maps_common_extensions() {
    CHK(mimeTypeFromExtension(".html") == "text/html");
    CHK(mimeTypeFromExtension(".css") == "text/css");
    CHK(mimeTypeFromExtension(".json") == "application/json");
}

// Verifies an extension not in the built-in table falls back to
// application/octet-stream.
static void falls_back_for_unknown_extension() {
    CHK(mimeTypeFromExtension(".unknownext") == "application/octet-stream");
}

// Verifies an empty extension falls back to application/octet-stream.
static void falls_back_for_empty_extension() {
    CHK(mimeTypeFromExtension("") == "application/octet-stream");
}

// Verifies matching is exact-case; an uppercase extension does not
// match a lowercase table entry.
static void matching_is_case_sensitive() {
    CHK(mimeTypeFromExtension(".HTML") != "text/html");
    CHK(mimeTypeFromExtension(".HTML") == "application/octet-stream");
}

// Executes all mimeTypeFromExtension() test cases.
static void run_tests() {
    RUN(maps_common_extensions);
    RUN(falls_back_for_unknown_extension);
    RUN(falls_back_for_empty_extension);
    RUN(matching_is_case_sensitive);
}

REGISTER_TEST_SUITE();
