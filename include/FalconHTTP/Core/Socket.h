#pragma once

// clang-format off
#include <atomic>  // std::atomic
#include <cstddef> // std::size_t, std::ptrdiff_t
#include <cstdint> // fixed-width integer types
#include <string>  // std::string
// clang-format on

// RAII wrapper around a single native TCP socket handle (POSIX file
// descriptor / Windows SOCKET). Move-only, IPv4 only. All operations
// report failure via return value rather than exceptions.

namespace FalconHTTP::Core {

/**
 * @class Socket
 * @brief Owns a single native TCP socket handle and exposes low-level,
 *        platform-independent socket operations.
 *
 * @details
 * Socket is move-only: copying a live socket handle would allow two
 * owners to independently close() the same underlying descriptor.
 * The destructor calls close() unconditionally, so a Socket going out
 * of scope always releases its handle - no manual cleanup is required.
 *
 * All operations report failure via a `bool`/negative return rather
 * than exceptions; none of Socket's members throw. Callers are
 * expected to check return values (`[[nodiscard]]`) since Socket does
 * not surface `errno`/`WSAGetLastError()` itself.
 *
 * @note IPv4 only. Socket::createTcp() always creates an `AF_INET`
 *       socket; there is currently no IPv6 (`AF_INET6`) support.
 */
class Socket {
  public:
    // Identity

    /// Sentinel value stored in fd_ for a socket that owns no handle
    /// (default-constructed, moved-from, or closed).
    static constexpr int invalidHandle = -1;

  private:
    // Storage
    std::atomic<int> fd_{invalidHandle};

  public:
    // Constructors & Destructor

    /// Constructs a Socket that owns no handle (isValid() == false).
    Socket() noexcept;

    /**
     * @brief Takes ownership of an existing native socket handle.
     * @param fd A native socket descriptor (e.g. the result of
     *        `::accept()`), or Socket::invalidHandle. Ownership
     *        transfers to this Socket; the caller must not close
     *        @p fd itself afterward.
     */
    explicit Socket(int fd) noexcept;

    /// Closes the owned handle, if any (equivalent to calling close()).
    ~Socket() noexcept;

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    /// Transfers ownership of @p other's handle; @p other becomes invalid.
    Socket(Socket&& other) noexcept;

    /// Closes any handle currently owned by *this, then takes ownership
    /// of @p other's handle; @p other becomes invalid.
    Socket& operator=(Socket&& other) noexcept;

    // Creation

    /**
     * @brief Creates a new IPv4 TCP socket (`socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)`).
     * @return A Socket owning the new handle, or a Socket with
     *         isValid() == false if creation failed.
     */
    [[nodiscard]] static Socket createTcp() noexcept;

    // Connection

    /**
     * @brief Connects this socket to a remote IPv4 TCP endpoint.
     *
     * @param address IPv4 address in dotted-decimal notation
     *        (for example, `"127.0.0.1"`).
     * @param port Destination TCP port.
     *
     * @return true if the connection was established successfully;
     *         false otherwise.
     *
     * @note The Socket must already own a valid TCP socket handle
     *       (typically created via createTcp()).
     */
    [[nodiscard]] bool connect(const std::string& address, uint16_t port) noexcept;

    // Socket Options

    /// Sets/clears `SO_REUSEADDR`. @return false if the underlying
    /// `setsockopt` call failed or this Socket is invalid.
    [[nodiscard]] bool setReuseAddr(bool enable) noexcept;

    /// Sets/clears non-blocking I/O mode (`O_NONBLOCK` / `FIONBIO`).
    /// @return false if the underlying call failed or this Socket is
    ///         invalid. @note Not currently used by Listener/Server -
    ///         the accept/connection-handling loop is fully blocking.
    [[nodiscard]] bool setNonBlocking(bool enable) noexcept;

    /// Sets/clears `TCP_NODELAY` (disables/enables Nagle's algorithm).
    /// @return false if the underlying `setsockopt` call failed or
    ///         this Socket is invalid.
    [[nodiscard]] bool setNoDelay(bool enable) noexcept;

    // I/O

    /**
     * @brief Writes up to @p length bytes from @p data to the socket.
     * @return Number of bytes actually written (may be less than
     *         @p length - a single call is not guaranteed to send
     *         everything), `0` if the peer closed the connection, or
     *         a negative value on error. Callers that need to send an
     *         exact byte count in full should loop on this (see
     *         Connection::sendAll).
     */
    [[nodiscard]] std::ptrdiff_t send(const void* data, std::size_t length) noexcept;

    /**
     * @brief Reads up to @p length bytes into @p buffer.
     * @return Number of bytes actually read, `0` if the peer closed
     *         the connection (EOF), or a negative value on error.
     *         @p buffer must be at least @p length bytes.
     */
    [[nodiscard]] std::ptrdiff_t receive(void* buffer, std::size_t length) noexcept;

    // Lifecycle

    /// Closes the owned handle if valid, and resets it to invalidHandle.
    /// Safe to call on an already-closed/invalid Socket (no-op).
    void close() noexcept;

    /// @return true if this Socket currently owns a handle
    ///         (fd_ != invalidHandle). Does not verify the handle is
    ///         still connected/usable at the OS level.
    [[nodiscard]] bool isValid() const noexcept;

    // Accessories

    /// @return The raw native handle, or invalidHandle if none is owned.
    [[nodiscard]] int handle() const noexcept;
};

} // namespace FalconHTTP::Core