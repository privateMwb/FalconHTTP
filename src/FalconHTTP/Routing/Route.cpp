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
    : method(method), pattern(std::move(pattern)), handler(std::move(handler)) {}

} // namespace FalconHTTP::Routing