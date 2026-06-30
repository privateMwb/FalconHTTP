#include "HTTP/HttpMethod.h"

namespace FalconHTTP::HTTP {

    // Conversion
    HttpMethod methodFromString(std::string_view text) noexcept {
        if (text == "GET")      return HttpMethod::Get;
        if (text == "POST")     return HttpMethod::Post;
        if (text == "PUT")      return HttpMethod::Put;
        if (text == "DELETE")   return HttpMethod::Delete;
        if (text == "PATCH")    return HttpMethod::Patch;
        if (text == "HEAD")     return HttpMethod::Head;
        if (text == "OPTIONS")  return HttpMethod::Options;
        
        return HttpMethod::Unknown;
    }

    std::string_view methodToString(HttpMethod method) noexcept {
        switch (method) {
            case HttpMethod::Get:      return "GET";
            case HttpMethod::Post:     return "POST";
            case HttpMethod::Put:      return "PUT";
            case HttpMethod::Delete:   return "DELETE";
            case HttpMethod::Patch:    return "PATCH";
            case HttpMethod::Head:     return "HEAD";
            case HttpMethod::Options:  return "OPTIONS";     
            case HttpMethod::Unknown:  return "UNKNOWN";
        }

        return "UNKNOWN";   
    }

} // namespace FalconHTTP::HTTP