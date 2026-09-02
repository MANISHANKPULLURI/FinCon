#pragma once

#include <string>

namespace fincon
{
    class LLMConfiguration
    {
    public:
        LLMConfiguration(
            std::string apiKey,
            std::string model
        );

        const std::string& apiKey() const;
        const std::string& model() const;

    private:
        std::string apiKey_;
        std::string model_;
    };
}