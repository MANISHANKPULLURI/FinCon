#pragma once

#include <string>

namespace fincon
{
    class LLMProvider
    {
    public:
        virtual ~LLMProvider() = default;

        virtual std::string generate(
            const std::string& request
        ) const = 0;
    };
}