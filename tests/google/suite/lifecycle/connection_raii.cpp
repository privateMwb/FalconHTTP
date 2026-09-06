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

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::Core;

// Verifies a default-constructed Connection is closed.
TEST(ConnectionRaii, DefaultConnectionIsClosed) {
    Connection connection;
    EXPECT_FALSE(connection.isOpen());
}

// Verifies wrapping a valid Socket produces an open Connection.
TEST(ConnectionRaii, WrapsValidSocketAsOpen) {
    Connection connection(Socket::createTcp());
    EXPECT_TRUE(connection.isOpen());
}

// Verifies move construction transfers the open state and leaves the
// source Connection closed.
TEST(ConnectionRaii, MoveConstructionTransfersOwnership) {
    Connection original(Socket::createTcp());
    ASSERT_TRUE(original.isOpen());

    Connection moved(std::move(original));

    EXPECT_TRUE(moved.isOpen());
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    EXPECT_FALSE(original.isOpen());
}

// Verifies move assignment transfers the open state into an
// already-open target and leaves the source Connection closed.
TEST(ConnectionRaii, MoveAssignmentTransfersOwnership) {
    Connection source(Socket::createTcp());
    Connection target(Socket::createTcp());
    ASSERT_TRUE(target.isOpen());

    target = std::move(source);

    EXPECT_TRUE(target.isOpen());

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    EXPECT_FALSE(source.isOpen());
}

// Verifies close() can be called more than once without error.
TEST(ConnectionRaii, CloseIsIdempotent) {
    Connection connection(Socket::createTcp());
    connection.close();
    EXPECT_FALSE(connection.isOpen());

    connection.close();
    EXPECT_FALSE(connection.isOpen());
}
