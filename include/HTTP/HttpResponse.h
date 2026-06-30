#pragma once

#include "HttpStatus.h"
#include "HashMap.h"
#include "Json.h"

#include <string>

namespace FalconHTTP::HTTP {
    class HttpResponse {
    private:

        // Storage
        HttpStatus                         status_ = HttpStatus::Ok;
        HashMap<std::string, std::string>  headers_;
        std::string                        body_;

    public:

        // Constructors
        HttpResponse() noexcept = default;
        explicit HttpResponse(HttpStatus status) noexcept;

        // Mutators
        void setStatus(HttpStatus status) noexcept;
        void setHeader(std::string name, std::string value);
        void setBody(std::string body) noexcept;
        void setJson(const Json& json);

        // Accessors
        [[nodiscard]] HttpStatus status() const noexcept;
        [[nodiscard]] const std::string& body() const noexcept;
        [[nodiscard]] bool hasHeader(const std::string& name) const noexcept;
        [[nodiscard]] std::string header(const std::string& name) const noexcept;
        [[nodiscard]] const HashMap<std::string, std::string>& headers() const noexcept;
    };

} // namespace FalconHTTP::HTTP