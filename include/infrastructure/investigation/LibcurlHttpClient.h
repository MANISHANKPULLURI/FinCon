#pragma once

#include "application/investigation/HttpClient.h"

namespace fincon
{
    class LibcurlHttpClient final : public HttpClient
    {
    public:
        std::string post(
            const std::string& url,
            const std::string& body,
            const std::string& authorization
        ) const override;
    };
}