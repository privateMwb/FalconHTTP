// Regression test: sendAll() must not report a socket error as
// success.
//
// The original bug (Connection.cpp): `if (sent == 0) return false;`
// only checked for exactly zero, missing the negative return
// Socket::send() gives on error. `totalSent +=
// static_cast<std::size_t>(sent)` with sent == -1 then underflowed to
// a huge value, making `totalSent < length` false and the loop exit
// - sendAll() returned true (success) on a socket error.
//
// This suite drives sendAll() directly through the public API by
// closing the underlying socket before calling it, which is
// sufficient to make the send() call fail without needing a live
// remote connection.

#include <FalconHTTP/FalconHTTP.h>
#include <gtest/gtest.h>

using namespace FalconHTTP::Core;

// Verifies sendAll() returns false when the underlying socket has
// already been closed - the send() call must fail, and that failure
// must be correctly reported rather than silently treated as success.
TEST(SendErrorReported, ReportsFailureOnClosedSocket) {
    Connection connection(Socket::createTcp());
    connection.close();

    const char data[] = "test";
    bool result = connection.sendAll(data, sizeof(data));

    EXPECT_FALSE(result);
}

// Verifies sendAll() on a freshly created, still-open (but
// unconnected) TCP socket does not silently report success either -
// an unconnected socket cannot actually deliver data, so send() on it
// should fail and sendAll() must reflect that.
TEST(SendErrorReported, ReportsFailureOnUnconnectedSocket) {
    Connection connection(Socket::createTcp());

    const char data[] = "test";
    bool result = connection.sendAll(data, sizeof(data));

    EXPECT_FALSE(result);
}
