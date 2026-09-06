/**
 * @file Route.cpp
 * @brief Route implementation.
 */

// clang-format off
#include <FalconHTTP/Routing/Route.h> // Route (own header)
// clang-format on

namespace FalconHTTP::Routing {

// Constructors
Route::Route(HTTP::HttpMethod method, std::string pattern, RouteHandler handler)
    : method(method), pattern(std::move(pattern)), kind(RouteKind::Normal),
      handler(std::move(handler)) {}

Route::Route(HTTP::HttpMethod method, std::string pattern, StreamHandler handler)
    : method(method), pattern(std::move(pattern)), kind(RouteKind::Stream),
      streamHandler(std::move(handler)) {}

} // namespace FalconHTTP::Routing