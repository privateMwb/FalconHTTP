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

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::Core;

// Verifies a default-constructed Socket owns no handle.
TEST(SocketRaii, DefaultConstructedSocketIsInvalid) {
    Socket socket;
    EXPECT_FALSE(socket.isValid());
    EXPECT_EQ(socket.handle(), Socket::invalidHandle);
}

// Verifies createTcp() produces a valid, owned handle.
TEST(SocketRaii, CreateTcpProducesValidSocket) {
    Socket socket = Socket::createTcp();
    EXPECT_TRUE(socket.isValid());
    EXPECT_NE(socket.handle(), Socket::invalidHandle);
}

// Verifies move construction transfers the underlying handle and
// leaves the source Socket owning nothing.
TEST(SocketRaii, MoveConstructionTransfersOwnership) {
    Socket original = Socket::createTcp();
    ASSERT_TRUE(original.isValid());
    int originalHandle = original.handle();

    Socket moved(std::move(original));

    EXPECT_TRUE(moved.isValid());
    EXPECT_EQ(moved.handle(), originalHandle);
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    EXPECT_FALSE(original.isValid());
}

// Verifies move assignment transfers the underlying handle into an
// already-valid target (closing what the target previously owned) and
// leaves the source Socket owning nothing.
TEST(SocketRaii, MoveAssignmentTransfersOwnership) {
    Socket source = Socket::createTcp();
    int sourceHandle = source.handle();

    Socket target = Socket::createTcp();
    ASSERT_TRUE(target.isValid());

    target = std::move(source);

    EXPECT_TRUE(target.isValid());
    EXPECT_EQ(target.handle(), sourceHandle);
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    EXPECT_FALSE(source.isValid());
}

// Verifies close() can be called more than once without error.
TEST(SocketRaii, CloseIsIdempotent) {
    Socket socket = Socket::createTcp();
    socket.close();
    EXPECT_FALSE(socket.isValid());

    socket.close();
    EXPECT_FALSE(socket.isValid());
}
