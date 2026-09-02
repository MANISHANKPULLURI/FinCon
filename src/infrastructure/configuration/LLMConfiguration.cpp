#include "application/investigation/LLMConfiguration.h"

#include <stdexcept>
#include <utility>

namespace fincon
{
    LLMConfiguration::LLMConfiguration(
        std::string apiKey,
        std::string model)
        : apiKey_(std::move(apiKey)),
          model_(std::move(model))
    {
        if (apiKey_.empty())
            throw std::invalid_argument("LLM API key is empty");

        if (model_.empty())
            throw std::invalid_argument("LLM model is empty");
    }

    const std::string& LLMConfiguration::apiKey() const
    {
        return apiKey_;
    }

    const std::string& LLMConfiguration::model() const
    {
        return model_;
    }
}