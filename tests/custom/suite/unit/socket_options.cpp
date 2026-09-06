// Socket option-setting test suite.
//
// Coverage:
// - setReuseAddr()/setNonBlocking()/setNoDelay() succeed on a valid,
//   freshly-created TCP socket
// - All three fail (return false) on a default-constructed
//   (invalid/no-handle) Socket
// - All three fail on a Socket after close()
// - createTcp() produces a valid socket; close() invalidates it

#include <support/framework.h>

using namespace FalconHTTP::Core;

// Verifies createTcp() produces a valid socket, and close()
// invalidates it.
static void create_and_close_toggle_validity() {
    Socket socket = Socket::createTcp();
    CHK(socket.isValid());

    socket.close();
    CHK(!socket.isValid());
}

// Verifies all three socket options succeed on a valid, freshly
// created TCP socket.
static void options_succeed_on_valid_socket() {
    Socket socket = Socket::createTcp();
    CHK(socket.isValid());

    CHK(socket.setReuseAddr(true));
    CHK(socket.setNonBlocking(true));
    CHK(socket.setNoDelay(true));
}

// Verifies all three socket options fail on a default-constructed
// (invalid) Socket, since there is no underlying handle to configure.
static void options_fail_on_invalid_socket() {
    Socket socket;
    CHK(!socket.isValid());

    CHK(!socket.setReuseAddr(true));
    CHK(!socket.setNonBlocking(true));
    CHK(!socket.setNoDelay(true));
}

// Verifies all three socket options fail once the socket has been
// explicitly closed.
static void options_fail_after_close() {
    Socket socket = Socket::createTcp();
    socket.close();

    CHK(!socket.setReuseAddr(true));
    CHK(!socket.setNonBlocking(true));
    CHK(!socket.setNoDelay(true));
}

// Executes all Socket option test cases.
static void run_tests() {
    RUN(create_and_close_toggle_validity);
    RUN(options_succeed_on_valid_socket);
    RUN(options_fail_on_invalid_socket);
    RUN(options_fail_after_close);
}

REGISTER_TEST_SUITE();
