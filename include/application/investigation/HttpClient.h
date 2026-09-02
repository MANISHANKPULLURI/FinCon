#pragma once

#include <string>

namespace fincon
{
    class HttpClient
    {
    public:
        virtual ~HttpClient() = default;

        virtual std::string post(
            const std::string& url,
            const std::string& body,
            const std::string& authorization
        ) const = 0;
    };
}