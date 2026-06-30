#pragma once

#include "Middleware.h"

namespace FalconHTTP::Middleware {

    class Recovery {
        public:

        // Core API
        void operator()(HTTP::HttpRequest& request, HTTP::HttpResponse& response, const NextHandler& next) const;
    };

} // namespace FalconHTTP::Middleware