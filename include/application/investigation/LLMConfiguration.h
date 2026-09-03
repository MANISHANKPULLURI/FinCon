#pragma once

#include <string>

namespace fincon
{
    class LLMConfiguration
    {
    public:
        LLMConfiguration(
            std::string apiKey,
            std::string model,
            std::string baseUrl = {}
        );

        const std::string& apiKey() const;
        const std::string& model() const;
        const std::string& baseUrl() const;

    private:
        std::string apiKey_;
        std::string model_;
        std::string baseUrl_;
    };
}