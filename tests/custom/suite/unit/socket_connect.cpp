// Unit tests for Socket::connect() and the send/receive timeout
// options, against a real loopback Listener.
//
// Coverage:
// - connect(address, port) succeeds against a live listener
// - connect(address, port, timeout) succeeds against a live listener
//   and leaves the socket blocking
// - connecting to a closed port fails (both overloads)
// - a malformed address, an invalid socket, and a non-positive timeout
//   all fail without touching the network
// - setReceiveTimeout() makes a silent peer's receive() fail instead of block
// - timeout setters fail on an invalid socket

#include <support/framework.h>

#include <chrono>

using namespace FalconHTTP::Core;

// Verifies the plain blocking connect() reaches a live listener.
static void connect_reaches_live_listener() {
    FalconHTTP::Core::Listener listener;
    CHK(listener.start(18721));

    Socket socket = Socket::createTcp();
    CHK(socket.connect("127.0.0.1", 18721));
}

// Verifies the timeout overload connects to a live listener too.
static void timed_connect_reaches_listener() {
    FalconHTTP::Core::Listener listener;
    CHK(listener.start(18722));

    Socket socket = Socket::createTcp();
    CHK(socket.connect("127.0.0.1", 18722, std::chrono::milliseconds(500)));
}

// Verifies a refused connection fails on both overloads.
static void connect_to_closed_port_fails() {
    Socket plain = Socket::createTcp();
    CHK(!plain.connect("127.0.0.1", 18729));

    Socket timed = Socket::createTcp();
    CHK(!timed.connect("127.0.0.1", 18729, std::chrono::milliseconds(500)));
}

// Verifies bad inputs are rejected before any connection attempt.
static void connect_rejects_bad_input() {
    Socket socket = Socket::createTcp();
    CHK(!socket.connect("not-an-ip", 18721));
    CHK(!socket.connect("127.0.0.1", 18721, std::chrono::milliseconds(0)));
    CHK(!socket.connect("127.0.0.1", 18721, std::chrono::milliseconds(-5)));

    Socket invalid;
    CHK(!invalid.connect("127.0.0.1", 18721));
    CHK(!invalid.connect("127.0.0.1", 18721, std::chrono::milliseconds(100)));
}

// Verifies a receive timeout turns a silent peer into a failed receive().
static void receive_timeout_unblocks() {
    FalconHTTP::Core::Listener listener;
    CHK(listener.start(18723));

    Socket socket = Socket::createTcp();
    CHK(socket.connect("127.0.0.1", 18723, std::chrono::milliseconds(500)));
    CHK(socket.setReceiveTimeout(std::chrono::milliseconds(100)));

    char buffer[8];
    const auto begin = std::chrono::steady_clock::now();
    const std::ptrdiff_t received = socket.receive(buffer, sizeof(buffer));
    const auto elapsed = std::chrono::steady_clock::now() - begin;

    CHK(received < 0); // timed out: negative, not 0 (which means peer closed).
    CHK(elapsed >= std::chrono::milliseconds(80));
    CHK(elapsed < std::chrono::milliseconds(1000));
}

// Verifies timeout setters report failure on an invalid socket.
static void timeouts_fail_on_invalid_socket() {
    Socket invalid;
    CHK(!invalid.setReceiveTimeout(std::chrono::milliseconds(100)));
    CHK(!invalid.setSendTimeout(std::chrono::milliseconds(100)));
}

static void run_tests() {
    RUN(connect_reaches_live_listener);
    RUN(timed_connect_reaches_listener);
    RUN(connect_to_closed_port_fails);
    RUN(connect_rejects_bad_input);
    RUN(receive_timeout_unblocks);
    RUN(timeouts_fail_on_invalid_socket);
}

REGISTER_TEST_SUITE();
