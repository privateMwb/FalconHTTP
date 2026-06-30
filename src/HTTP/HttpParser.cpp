#include "HTTP/HttpParser.h"
#include "HTTP/HttpMethod.h"
#include "Utility/UrlDecoder.h"

#include <stdexcept>

namespace FalconHTTP::HTTP {

    // Core API
    HttpRequest HttpParser::parse(std::string_view raw) {
        HttpRequest request;
        std::string_view remaining = raw;

        parseRequestLine(extractLine(remaining), request);

        while (true) {
            std::string_view line = extractLine(remaining);
            if (line.empty()) break;
            parseHeaderLine(line, request);
        }

        request.setBody(std::string(remaining));

        return request;
    }

    // Private Helper
    std::string_view HttpParser::extractLine(std::string_view& remaining) {
        std::size_t pos = remaining.find("\r\n");
        if (pos == std::string_view::npos) {
            throw std::runtime_error("HttpParser: malformed request, missing CRLF");
        }

        std::string_view line = remaining.substr(0, pos);
        remaining.remove_prefix(pos + 2);
        return line;
    }

    void HttpParser::parseRequestLine(std::string_view line, HttpRequest& request) {
        std::size_t firstSpace = line.find(' ');
        std::size_t secondSpace = line.find(' ', firstSpace + 1);

        if (firstSpace == std::string_view::npos || secondSpace == std::string_view::npos) {
            throw std::runtime_error("HttpParser: malformed request line");
        }

        std::string_view methodText = line.substr(0, firstSpace);
        std::string_view rawPath = line.substr(firstSpace + 1, secondSpace - firstSpace - 1);
        std::string_view versionText = line.substr(secondSpace + 1);

        request.setMethod(methodFromString(methodText));
        parsePath(rawPath, request);
        request.setVersion(std::string(versionText));
    }

    void HttpParser::parsePath(std::string_view rawPath, HttpRequest& request) {
        std::size_t queryStart = rawPath.find('?');

        if (queryStart == std::string_view::npos) {
            request.setPath(std::string(rawPath));
            return;
        }

        request.setPath(std::string(rawPath.substr(0, queryStart)));

        std::string_view query = rawPath.substr(queryStart + 1);
        while (!query.empty()) {
            std::size_t ampersand = query.find('&');
            std::string_view pair = (ampersand == std::string_view::npos) ? query : query.substr(0, ampersand);

            std::size_t equals = pair.find('=');
            if (equals != std::string_view::npos) {
                std::string key = Utility::UrlDecoder::decode(pair.substr(0, equals));
                std::string value = Utility::UrlDecoder::decode(pair.substr(equals + 1));
                request.setQueryParam(std::move(key), std::move(value));
            }
            else if (!pair.empty()) {
                std::string key = Utility::UrlDecoder::decode(pair);
                request.setQueryParam(std::move(key), "");
            }

            if (ampersand == std::string_view::npos) break;
            query.remove_prefix(ampersand + 1);
        }
    }

    void HttpParser::parseHeaderLine(std::string_view line, HttpRequest& request) {
        std::size_t colon = line.find(':');
        if (colon == std::string_view::npos) {
            throw std::runtime_error("HttpParser:: malformed header line");
        }

        std::string_view name = line.substr(0, colon);
        std::string_view value = line.substr(colon + 1);

        while (!value.empty() && value.front() == ' ') value.remove_prefix(1);

        request.setHeader(std::string(name), std::string(value));
    }

}
