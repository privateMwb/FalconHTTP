#include "../../include/HTTP/MimeTypes.h"

namespace FalconHTTP::HTTP {

    // Lookup
    std::string_view mimeTypeFromExtension(std::string_view extension) noexcept {
        if (extension == ".html" || extension == ".htm")  return "text/html";
        if (extension == ".css")                          return "text/css";
        if (extension == ".js")                           return "application/javascript";
        if (extension == ".json")                         return "application/json";
        if (extension == ".png")                          return "image/png";
        if (extension == ".jpg" || extension == ".jpeg")  return "image/jpeg";
        if (extension == ".gif")                          return "image/gif";
        if (extension == ".svg")                          return "image/svg+xml";
        if (extension == ".ico")                          return "image/x-icon";
        if (extension == ".txt")                          return "text/plain";
        if (extension == ".pdf")                          return "application/pdf";

        return "application/octet-stream";
    }

} // namespace FalconHTTP::HTTP