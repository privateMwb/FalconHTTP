#include "HTTP/HttpSerializer.h"
#include "HTTP/HttpStatus.h"

namespace FalconHTTP::HTTP {
    
    // Core API
    std::string HttpSerializer::serialize(const HttpResponse& response) {
        std::string output;

        appendStatusLine(output, response);
        appendHeaders(output, response);
        output += response.body();

        return output;
    }

    // Private Helpers
    void HttpSerializer::appendStatusLine(std::string& output, const HttpResponse& response) {
        output += "HTTP/1.1 ";
        output += std::to_string(static_cast<int>(response.status()));
        output += " ";
        output += statusReasonPhrase(response.status());
        output += "\r\n";
    }

    void HttpSerializer::appendHeaders(std::string& output, const HttpResponse& response) {
        for (const auto& entry : response.headers()) {
            output += entry.key;
            output += ": ";
            output += entry.value;
            output += "\r\n";
        }

        output += "Content-Length: ";
        output += std::to_string(response.body().size());
        output += "\r\n";

        output += "\r\n";
    }
    
} // namespace FalconHTTP::HTTP