#pragma once

#include "application/investigation/HttpClient.h"
#include "application/investigation/LLMProvider.h"

#include <string>

namespace fincon
{
    class MetaLlamaLLMProvider final : public LLMProvider
    {
    public:
        MetaLlamaLLMProvider(
            const HttpClient& httpClient,
            std::string apiKey,
            std::string model
        );

        std::string generate(
            const std::string& request
        ) const override;

    private:
        const HttpClient& httpClient_;
        std::string apiKey_;
        std::string model_;
    };
}