#pragma once

#include <string>
#include <utility>

namespace fincon
{

    enum class ToolCallStatus
    {
        Requested,
        Running,
        Succeeded,
        Failed
    };

    class InvestigationToolCall final
    {
    public:

        InvestigationToolCall() = default;

        explicit InvestigationToolCall(
            std::string id)
            : id_(std::move(id))
        {
        }

        const std::string& id() const
        {
            return id_;
        }

        const std::string& toolName() const
        {
            return toolName_;
        }

        const std::string& input() const
        {
            return input_;
        }

        const std::string& result() const
        {
            return result_;
        }

        ToolCallStatus status() const
        {
            return status_;
        }

        void setToolName(
            std::string toolName)
        {
            toolName_ =
                std::move(toolName);
        }

        void setInput(
            std::string input)
        {
            input_ =
                std::move(input);
        }

        void setResult(
            std::string result)
        {
            result_ =
                std::move(result);
        }

        void setStatus(
            ToolCallStatus status)
        {
            status_ = status;
        }

    private:

        std::string id_;

        std::string toolName_;

        std::string input_;

        std::string result_;

        ToolCallStatus status_ =
            ToolCallStatus::Requested;
    };

}