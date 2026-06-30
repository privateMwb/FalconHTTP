#include "HTTP/HttpStatus.h"

namespace FalconHTTP::HTTP {
    
    // Conversion
    std::string_view statusReasonPhrase(HttpStatus status) noexcept {
        switch (status) {
            case HttpStatus::Ok:                   return "OK";
            case HttpStatus::Created:              return "Created";
            case HttpStatus::NoContent:            return "No Content";
            case HttpStatus::BadRequest:           return "Bad Request";
            case HttpStatus::Unauthorized:         return "Unauthorized";
            case HttpStatus::Forbidden:            return "Forbidden";
            case HttpStatus::NotFound:             return "Not Found";
            case HttpStatus::MethodNotAllowed:     return "Method Not Allowed";
            case HttpStatus::InternalServerError:  return "Internal Server Error";
            case HttpStatus::NotImplemented:       return "Not Implemented";
        }

        return "Unknown";
    }

} // namespace FalconHTTP::HTTP