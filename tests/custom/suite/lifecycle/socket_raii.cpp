// Socket RAII lifecycle test suite.
//
// Coverage:
// - A default-constructed Socket owns no handle
// - createTcp() produces a valid, owned handle
// - Move construction transfers ownership; the moved-from Socket
//   becomes invalid
// - Move assignment transfers ownership (closing whatever the target
//   previously owned); the moved-from Socket becomes invalid
// - close() is safe to call more than once (idempotent)

#include <support/framework.h>

using namespace FalconHTTP::Core;

// Verifies a default-constructed Socket owns no handle.
static void default_constructed_socket_is_invalid() {
    Socket socket;
    CHK(!socket.isValid());
    CHK(socket.handle() == Socket::invalidHandle);
}

// Verifies createTcp() produces a valid, owned handle.
static void create_tcp_produces_valid_socket() {
    Socket socket = Socket::createTcp();
    CHK(socket.isValid());
    CHK(socket.handle() != Socket::invalidHandle);
}

// Verifies move construction transfers the underlying handle and
// leaves the source Socket owning nothing.
static void move_construction_transfers_ownership() {
    Socket original = Socket::createTcp();
    CHK(original.isValid());
    int originalHandle = original.handle();

    Socket moved(std::move(original));

    CHK(moved.isValid());
    CHK(moved.handle() == originalHandle);
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    CHK(!original.isValid());
}

// Verifies move assignment transfers the underlying handle into an
// already-valid target (closing what the target previously owned) and
// leaves the source Socket owning nothing.
static void move_assignment_transfers_ownership() {
    Socket source = Socket::createTcp();
    int sourceHandle = source.handle();

    Socket target = Socket::createTcp();
    CHK(target.isValid());

    target = std::move(source);

    CHK(target.isValid());
    CHK(target.handle() == sourceHandle);
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    CHK(!source.isValid());
}

// Verifies close() can be called more than once without error.
static void close_is_idempotent() {
    Socket socket = Socket::createTcp();
    socket.close();
    CHK(!socket.isValid());

    socket.close();
    CHK(!socket.isValid());
}

// Executes all Socket RAII test cases.
static void run_tests() {
    RUN(default_constructed_socket_is_invalid);
    RUN(create_tcp_produces_valid_socket);
    RUN(move_construction_transfers_ownership);
    RUN(move_assignment_transfers_ownership);
    RUN(close_is_idempotent);
}

REGISTER_TEST_SUITE();
