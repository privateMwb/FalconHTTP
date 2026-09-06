// Listener bind/close lifecycle test suite.
//
// Coverage:
// - A default-constructed Listener is not listening
// - start() succeeds and reports listening
// - stop() closes the listener and resets port() to 0
// - Repeated start/stop cycles on the same Listener all succeed
//
// NOTE: all tests bind to port 0 so the kernel assigns an ephemeral
// port, avoiding collisions with anything else using a fixed port on
// the test machine. Per Listener::port()'s documented contract, this
// means port() returns the literal value passed to start() (0), not
// the kernel-assigned port - which is indistinguishable from "not
// currently listening" (port() also returns 0 in that case). This is
// a real ambiguity in the current API, not a test bug; if a future
// caller actually needs the OS-assigned ephemeral port number, this
// contract will need to change.

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

// VectorPro (pulled in transitively through Router.h's `using
// namespace VectorPro;`) declares a concept also named `Listener`
// (VectorPro::Listener). Router.h's using-directive puts that name at
// global scope, so ANY unqualified `Listener` at global scope in this
// file - including one introduced via a using-declaration - lands in
// that same scope and stays ambiguous; a using-declaration only
// shadows a using-directive from an *enclosing* scope, not one that's
// already at the same scope. The only reliable fix is to never
// introduce an unqualified `Listener` name at all: this alias gives
// the type a distinct local name instead.
using TestListener = FalconHTTP::Core::Listener;

// Verifies a default-constructed Listener is not listening and
// reports port 0.
TEST(ListenerBindClose, DefaultListenerIsNotListening) {
    TestListener listener;
    EXPECT_FALSE(listener.isListening());
    EXPECT_EQ(listener.port(), 0);
}

// Verifies start() succeeds and isListening() reports true afterward.
TEST(ListenerBindClose, StartSucceedsAndReportsListening) {
    TestListener listener;
    ASSERT_TRUE(listener.start(0));
    EXPECT_TRUE(listener.isListening());
}

// Verifies stop() closes the listener and resets port() to 0.
TEST(ListenerBindClose, StopClosesAndResetsPort) {
    TestListener listener;
    ASSERT_TRUE(listener.start(0));
    ASSERT_TRUE(listener.isListening());

    listener.stop();

    EXPECT_FALSE(listener.isListening());
    EXPECT_EQ(listener.port(), 0);
}

// Verifies the same Listener can go through multiple start/stop
// cycles, each succeeding independently.
TEST(ListenerBindClose, RepeatedStartStopCyclesSucceed) {
    TestListener listener;

    for (int i = 0; i < 3; ++i) {
        ASSERT_TRUE(listener.start(0));
        ASSERT_TRUE(listener.isListening());

        listener.stop();
        EXPECT_FALSE(listener.isListening());
    }
}
