#pragma once

#include "HTTP/HttpRequest.h"
#include "HTTP/HttpResponse.h"
#include "function/Function.h"

namespace FalconHTTP::Routing {

    // Alias
    using RouteHandler = FunctionPro::Function<void(const HTTP::HttpRequest&, HTTP::HttpResponse&)>;

} // namespace FalconHTTP::Routing
