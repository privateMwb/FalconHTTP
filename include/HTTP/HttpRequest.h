#pragma once

#include "HttpMethod.h"
#include "HashMap.h"
#include "Json.h"

#include <string>

namespace FalconHTTP::HTTP {

    class HttpRequest {
    public:

        // Identity
        static constexpr std::string_view defaultVersion = "HTTP/1.1";

    private:

        // Storage
        HttpMethod                         method_ = HttpMethod::Unknown;
        std::string                        path_;
        std::string                        version_;
        HashMap<std::string, std::string>  headers_;
        HashMap<std::string, std::string>  queryParams_;
        HashMap<std::string, std::string>  pathParams_;
        std::string                        body_;

    public:

        // Constructor
        HttpRequest() noexcept = default;

        // Mutators
        void setMethod(HttpMethod method)     noexcept;
        void setPath(std::string path)        noexcept;
        void setVersion(std::string version)  noexcept;
        void setHeader(std::string name, std::string value);
        void setQueryParam(std::string name, std::string value);
        void setPathParam(std::string name, std::string value);
        void setBody(std::string body)        noexcept;

        // Accessors
        [[nodiscard]] HttpMethod          method()   const noexcept;
        [[nodiscard]] const std::string& path()     const noexcept;
        [[nodiscard]] const std::string& version()  const noexcept;
        [[nodiscard]] const std::string& body()     const noexcept;
        [[nodiscard]] bool                hasHeader(const std::string& name)      const noexcept;
        [[nodiscard]] std::string         header(const std::string& name)         const noexcept;
        [[nodiscard]] bool                hasQueryParam(const std::string& name)  const noexcept;
        [[nodiscard]] std::string         queryParam(const std::string& name)     const noexcept;
        [[nodiscard]] bool                hasPathParam(const std::string& name)   const noexcept;
        [[nodiscard]] std::string         pathParam(const std::string& name)      const noexcept;

        // JSON
        [[nodiscard]] Json json() const;
    };

} // namespace FalconHTTP::HTTP