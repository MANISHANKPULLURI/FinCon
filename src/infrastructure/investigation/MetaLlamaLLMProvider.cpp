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
        std::string model,
        std::string baseUrl)
        : httpClient_(httpClient),
          apiKey_(std::move(apiKey)),
          model_(std::move(model)),
          baseUrl_(std::move(baseUrl))
    {
        if (model_.empty())
            model_ = "muse-spark-1.2-contributor";
        if (baseUrl_.empty())
            baseUrl_ = "https://api.muse.ai/v1";
        while (baseUrl_.size() > 1 && baseUrl_.back() == '/')
            baseUrl_.pop_back();
    }

    std::string MetaLlamaLLMProvider::generate(
        const std::string& request) const
    {
        if (apiKey_.empty())
            throw std::runtime_error("LLM API key not configured, skipping LLM escalation");

        if (request.empty())
            throw std::invalid_argument("LLM request is empty");

        std::string endpoint;
        nlohmann::json body;
        if (baseUrl_.find("anthropic.com") != std::string::npos)
        {
            if (baseUrl_.find("/v1") != std::string::npos)
                endpoint = baseUrl_ + "/messages";
            else
                endpoint = baseUrl_ + "/v1/messages";
            body = {
                {"model", model_},
                {"max_tokens", 2048},
                {"messages", {{{"role", "user"}, {"content", request}}}}
            };
        }
        else
        {
            if (baseUrl_.find("/chat/completions") != std::string::npos)
                endpoint = baseUrl_;
            else if (baseUrl_.find("/v1") != std::string::npos)
                endpoint = baseUrl_ + "/chat/completions";
            else
                endpoint = baseUrl_ + "/v1/chat/completions";
            body = {
                {"model", model_},
                {"messages", {{{"role", "user"}, {"content", request}}}}
            };
        }

        const std::string response =
            httpClient_.post(
                endpoint,
                body.dump(),
                apiKey_
            );

        if (response.empty())
            throw std::runtime_error(
                "LLM API returned an empty response"
            );

        const nlohmann::json jsonResponse =
            nlohmann::json::parse(response);

        if (!jsonResponse.is_object())
            throw std::runtime_error(
                "LLM API response must be a JSON object"
            );

        std::string content;
        if (jsonResponse.contains("content") && jsonResponse["content"].is_array() && !jsonResponse["content"].empty())
        {
            const auto& c0 = jsonResponse["content"][0];
            if (c0.is_object() && c0.contains("text") && c0["text"].is_string())
                content = c0["text"].get<std::string>();
            else if (c0.is_string())
                content = c0.get<std::string>();
        }
        if (content.empty())
        {
            if (!jsonResponse.contains("choices") ||
                !jsonResponse["choices"].is_array() ||
                jsonResponse["choices"].empty())
            {
                throw std::runtime_error(
                    "LLM API response contains no choices/content"
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
                    "LLM API response contains no assistant content"
                );
            }
            content = choice["message"]["content"].get<std::string>();
        }

        if (content.empty())
            throw std::runtime_error(
                "LLM API returned empty assistant content"
            );

        std::cerr << "\n--- MUSE RAW CONTENT ---\n"
                  << content
                  << "\n--- END MUSE RAW CONTENT ---\n";

        return content;
    }
}