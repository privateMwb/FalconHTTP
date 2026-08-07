// Feeding HttpParser malformed input.
//
// Demonstrates:
// - parse() throwing std::runtime_error for each of its three documented
//   failure cases: missing CRLF, a request line without both a method and
//   a version, and a header line without a colon
// - What one of those exceptions actually becomes if it reaches Server
//   uncaught: a generic 500 Internal Server Error, not a 400 Bad Request -
//   Server's top-level catch (see Server.cpp) treats every std::exception
//   the same way, regardless of whether the client or the server was at fault

#include <support/framework.h>

#include <iostream>
#include <stdexcept>

using namespace FalconHTTP;

static void run_examples() {

    // No CRLF at all: extractLine() can't find a line terminator for even
    // the request line.
    setTitle("Missing CRLF");

    try {
        (void)HTTP::HttpParser::parse("GET / HTTP/1.1");
        std::cout << "unreachable\n";
    } catch (const std::exception& e) {
        std::cout << "threw: " << e.what() << "\n";
    }

    std::cout << "\n";

    // A request line with only one space-separated field can't be split
    // into method/path/version.
    setTitle("Malformed Request Line");

    try {
        (void)HTTP::HttpParser::parse("GET\r\n\r\n");
        std::cout << "unreachable\n";
    } catch (const std::exception& e) {
        std::cout << "threw: " << e.what() << "\n";
    }

    std::cout << "\n";

    // A header line with no colon can't be split into name/value.
    setTitle("Malformed Header Line");

    try {
        (void)HTTP::HttpParser::parse("GET / HTTP/1.1\r\nNotAHeader\r\n\r\n");
        std::cout << "unreachable\n";
    } catch (const std::exception& e) {
        std::cout << "threw: " << e.what() << "\n";
    }

    std::cout << "\n";

    // If a caller doesn't wrap parse() in its own try/catch, Server's
    // generic top-level catch is what actually handles it - and it can't
    // tell a malformed *request* apart from a genuine server-side bug, so
    // both end up as the same 500 response.
    setTitle("What Server Does With It");

    std::cout << "an uncaught parse() failure inside Server::handleConnection()\n";
    std::cout << "becomes: 500 Internal Server Error (not 400 Bad Request)\n";
}

REGISTER_EXAMPLE_SUITE();
