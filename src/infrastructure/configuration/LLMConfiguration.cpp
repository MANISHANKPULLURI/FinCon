#include "application/investigation/LLMConfiguration.h"

#include <stdexcept>
#include <utility>

namespace fincon
{
    LLMConfiguration::LLMConfiguration(
        std::string apiKey,
        std::string model,
        std::string baseUrl)
        : apiKey_(std::move(apiKey)),
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

    const std::string& LLMConfiguration::apiKey() const
    {
        return apiKey_;
    }

    const std::string& LLMConfiguration::model() const
    {
        return model_;
    }

    const std::string& LLMConfiguration::baseUrl() const
    {
        return baseUrl_;
    }
}