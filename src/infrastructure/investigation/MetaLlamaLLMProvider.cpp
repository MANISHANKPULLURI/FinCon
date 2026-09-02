#include "infrastructure/investigation/MetaLlamaLLMProvider.h"

#include <nlohmann/json.hpp>

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace fincon
{
    MetaLlamaLLMProvider::MetaLlamaLLMProvider(
        const HttpClient& httpClient,
        std::string apiKey,
        std::string model)
        : httpClient_(httpClient),
          apiKey_(std::move(apiKey)),
          model_(std::move(model))
    {
        if (apiKey_.empty())
            throw std::invalid_argument("Meta Model API key is empty");

        if (model_.empty())
            throw std::invalid_argument("Meta Model API model is empty");
    }

    std::string MetaLlamaLLMProvider::generate(
        const std::string& request) const
    {
        if (request.empty())
            throw std::invalid_argument("LLM request is empty");

        const nlohmann::json body = {
            {"model", model_},
            {"messages", {
                {
                    {"role", "user"},
                    {"content", request}
                }
            }}
        };

        const std::string response =
            httpClient_.post(
                "https://api.meta.ai/v1/chat/completions",
                body.dump(),
                "Bearer " + apiKey_
            );

        if (response.empty())
            throw std::runtime_error(
                "Meta Model API returned an empty response"
            );

        const nlohmann::json jsonResponse =
            nlohmann::json::parse(response);

        if (!jsonResponse.is_object())
            throw std::runtime_error(
                "Meta Model API response must be a JSON object"
            );

        if (!jsonResponse.contains("choices") ||
            !jsonResponse["choices"].is_array() ||
            jsonResponse["choices"].empty())
        {
            throw std::runtime_error(
                "Meta Model API response contains no choices"
            );
        }

        const auto& choice = jsonResponse["choices"][0];

        if (!choice.is_object() ||
            !choice.contains("message") ||
            !choice["message"].is_object() ||
            !choice["message"].contains("content") ||
            !choice["message"]["content"].is_string())
        {
            throw std::runtime_error(
                "Meta Model API response contains no assistant content"
            );
        }

        const std::string content =
            choice["message"]["content"].get<std::string>();

        if (content.empty())
            throw std::runtime_error(
                "Meta Model API returned empty assistant content"
            );

        std::cerr << "\n--- MUSE RAW CONTENT ---\n"
                  << content
                  << "\n--- END MUSE RAW CONTENT ---\n";

        return content;
    }
}