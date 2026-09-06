// Socket option-setting test suite.
//
// Coverage:
// - setReuseAddr()/setNonBlocking()/setNoDelay() succeed on a valid,
//   freshly-created TCP socket
// - All three fail (return false) on a default-constructed
//   (invalid/no-handle) Socket
// - All three fail on a Socket after close()
// - createTcp() produces a valid socket; close() invalidates it

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::Core;

// Verifies createTcp() produces a valid socket, and close()
// invalidates it.
TEST(SocketOptions, CreateAndCloseToggleValidity) {
    Socket socket = Socket::createTcp();
    ASSERT_TRUE(socket.isValid());

    socket.close();
    EXPECT_FALSE(socket.isValid());
}

// Verifies all three socket options succeed on a valid, freshly
// created TCP socket.
TEST(SocketOptions, OptionsSucceedOnValidSocket) {
    Socket socket = Socket::createTcp();
    ASSERT_TRUE(socket.isValid());

    EXPECT_TRUE(socket.setReuseAddr(true));
    EXPECT_TRUE(socket.setNonBlocking(true));
    EXPECT_TRUE(socket.setNoDelay(true));
}

// Verifies all three socket options fail on a default-constructed
// (invalid) Socket, since there is no underlying handle to configure.
TEST(SocketOptions, OptionsFailOnInvalidSocket) {
    Socket socket;
    ASSERT_FALSE(socket.isValid());

    EXPECT_FALSE(socket.setReuseAddr(true));
    EXPECT_FALSE(socket.setNonBlocking(true));
    EXPECT_FALSE(socket.setNoDelay(true));
}

// Verifies all three socket options fail once the socket has been
// explicitly closed.
TEST(SocketOptions, OptionsFailAfterClose) {
    Socket socket = Socket::createTcp();
    socket.close();

    EXPECT_FALSE(socket.setReuseAddr(true));
    EXPECT_FALSE(socket.setNonBlocking(true));
    EXPECT_FALSE(socket.setNoDelay(true));
}
