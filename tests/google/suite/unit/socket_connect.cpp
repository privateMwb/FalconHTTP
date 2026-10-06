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

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

#include <chrono>

using namespace FalconHTTP::Core;

// Verifies the plain blocking connect() reaches a live listener.
TEST(SocketConnect, ConnectReachesLiveListener) {
    FalconHTTP::Core::Listener listener;
    ASSERT_TRUE(listener.start(18721));

    Socket socket = Socket::createTcp();
    EXPECT_TRUE(socket.connect("127.0.0.1", 18721));
}

// Verifies the timeout overload connects to a live listener too.
TEST(SocketConnect, TimedConnectReachesListener) {
    FalconHTTP::Core::Listener listener;
    ASSERT_TRUE(listener.start(18722));

    Socket socket = Socket::createTcp();
    EXPECT_TRUE(socket.connect("127.0.0.1", 18722, std::chrono::milliseconds(500)));
}

// Verifies a refused connection fails on both overloads.
TEST(SocketConnect, ConnectToClosedPortFails) {
    Socket plain = Socket::createTcp();
    EXPECT_FALSE(plain.connect("127.0.0.1", 18729));

    Socket timed = Socket::createTcp();
    EXPECT_FALSE(timed.connect("127.0.0.1", 18729, std::chrono::milliseconds(500)));
}

// Verifies bad inputs are rejected before any connection attempt.
TEST(SocketConnect, ConnectRejectsBadInput) {
    Socket socket = Socket::createTcp();
    EXPECT_FALSE(socket.connect("not-an-ip", 18721));
    EXPECT_FALSE(socket.connect("127.0.0.1", 18721, std::chrono::milliseconds(0)));
    EXPECT_FALSE(socket.connect("127.0.0.1", 18721, std::chrono::milliseconds(-5)));

    Socket invalid;
    EXPECT_FALSE(invalid.connect("127.0.0.1", 18721));
    EXPECT_FALSE(invalid.connect("127.0.0.1", 18721, std::chrono::milliseconds(100)));
}

// Verifies a receive timeout turns a silent peer into a failed receive().
TEST(SocketConnect, ReceiveTimeoutUnblocks) {
    FalconHTTP::Core::Listener listener;
    ASSERT_TRUE(listener.start(18723));

    Socket socket = Socket::createTcp();
    EXPECT_TRUE(socket.connect("127.0.0.1", 18723, std::chrono::milliseconds(500)));
    EXPECT_TRUE(socket.setReceiveTimeout(std::chrono::milliseconds(100)));

    char buffer[8];
    const auto begin = std::chrono::steady_clock::now();
    const std::ptrdiff_t received = socket.receive(buffer, sizeof(buffer));
    const auto elapsed = std::chrono::steady_clock::now() - begin;

    EXPECT_TRUE(received < 0); // timed out: negative, not 0 (which means peer closed).
    EXPECT_TRUE(elapsed >= std::chrono::milliseconds(80));
    EXPECT_TRUE(elapsed < std::chrono::milliseconds(1000));
}

// Verifies timeout setters report failure on an invalid socket.
TEST(SocketConnect, TimeoutsFailOnInvalidSocket) {
    Socket invalid;
    EXPECT_FALSE(invalid.setReceiveTimeout(std::chrono::milliseconds(100)));
    EXPECT_FALSE(invalid.setSendTimeout(std::chrono::milliseconds(100)));
}
