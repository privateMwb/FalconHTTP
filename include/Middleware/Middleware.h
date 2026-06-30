#pragma once

#include "HTTP/HttpRequest.h"
#include "HTTP/HttpResponse.h"
#include "function/Function.h"

namespace FalconHTTP::Middleware {

    // Alias
    using NextHandler   = FunctionPro::Function<void(HTTP::HttpRequest&, HTTP::HttpResponse&)>;
    using MiddlewareFn  = FunctionPro::Function<void(HTTP::HttpRequest&, HTTP::HttpResponse&, const NextHandler&)>;

} // namespace FalconHTTP::Middleware