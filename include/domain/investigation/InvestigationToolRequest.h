#pragma once

#include <string>
#include <utility>

namespace fincon
{
    class InvestigationToolRequest
    {
    public:
        InvestigationToolRequest() = default;

        InvestigationToolRequest(
            std::string toolName,
            std::string input)
            : toolName_(std::move(toolName)),
              input_(std::move(input))
        {
        }

        const std::string& toolName() const
        {
            return toolName_;
        }

        const std::string& input() const
        {
            return input_;
        }

        void setToolName(std::string toolName)
        {
            toolName_ = std::move(toolName);
        }

        void setInput(std::string input)
        {
            input_ = std::move(input);
        }

    private:
        std::string toolName_;
        std::string input_;
    };
}