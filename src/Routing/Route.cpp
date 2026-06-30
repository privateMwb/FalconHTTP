#include "Routing/Route.h"

namespace FalconHTTP::Routing {

// Constructors
Route::Route(HTTP::HttpMethod method, std::string pattern, RouteHandler handler)
	: method(method), pattern(std::move(pattern)), handler(std::move(handler)) {}

} // namespace FalconHTTP::Routing