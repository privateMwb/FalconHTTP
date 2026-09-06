// Connection RAII lifecycle test suite.
//
// Coverage:
// - A default-constructed Connection is closed
// - Wrapping a valid Socket produces an open Connection
// - Move construction transfers ownership; the moved-from Connection
//   becomes closed
// - Move assignment transfers ownership; the moved-from Connection
//   becomes closed
// - close() is safe to call more than once (idempotent)

#include <support/framework.h>

using namespace FalconHTTP::Core;

// Verifies a default-constructed Connection is closed.
static void default_connection_is_closed() {
    Connection connection;
    CHK(!connection.isOpen());
}

// Verifies wrapping a valid Socket produces an open Connection.
static void wraps_valid_socket_as_open() {
    Connection connection(Socket::createTcp());
    CHK(connection.isOpen());
}

// Verifies move construction transfers the open state and leaves the
// source Connection closed.
static void move_construction_transfers_ownership() {
    Connection original(Socket::createTcp());
    CHK(original.isOpen());

    Connection moved(std::move(original));

    CHK(moved.isOpen());
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    CHK(!original.isOpen());
}

// Verifies move assignment transfers the open state into an
// already-open target and leaves the source Connection closed.
static void move_assignment_transfers_ownership() {
    Connection source(Socket::createTcp());
    Connection target(Socket::createTcp());
    CHK(target.isOpen());

    target = std::move(source);

    CHK(target.isOpen());

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    CHK(!source.isOpen());
}

// Verifies close() can be called more than once without error.
static void close_is_idempotent() {
    Connection connection(Socket::createTcp());
    connection.close();
    CHK(!connection.isOpen());

    connection.close();
    CHK(!connection.isOpen());
}

// Executes all Connection RAII test cases.
static void run_tests() {
    RUN(default_connection_is_closed);
    RUN(wraps_valid_socket_as_open);
    RUN(move_construction_transfers_ownership);
    RUN(move_assignment_transfers_ownership);
    RUN(close_is_idempotent);
}

REGISTER_TEST_SUITE();
