#pragma once

#include <string_view>

namespace FalconHTTP::HTTP {

    // Codes
    enum class HttpStatus {
        Ok                   = 200,
        Created              = 201,
        NoContent            = 204,
        BadRequest           = 400,
        Unauthorized         = 401,
        Forbidden            = 403,
        NotFound             = 404,
        MethodNotAllowed     = 405,
        InternalServerError  = 500,
        NotImplemented       = 501
    };
    
    // Conversion
    [[nodiscard]] std::string_view statusReasonPhrase(HttpStatus status) noexcept; 

} // namespace FalconHTTP::HTTP