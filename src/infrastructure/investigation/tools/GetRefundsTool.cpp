#include "infrastructure/investigation/tools/GetRefundsTool.h"

#include <string>
#include <vector>

namespace fincon
{
    GetRefundsTool::GetRefundsTool(
        const FinancialDataRepository& repository)
        : repository_(repository)
    {
    }

    InvestigationToolCall GetRefundsTool::execute(
        const InvestigationToolCall& request) const
    {
        InvestigationToolCall result = request;

        result.setStatus(ToolCallStatus::Running);

        const std::string& paymentId =
            request.input();

        const std::vector<const Refund*> refunds =
            repository_.getRefunds(paymentId);

        if (refunds.empty())
        {
            result.setStatus(ToolCallStatus::Succeeded);

            result.setResult(
                "No refunds found for payment: " + paymentId
            );

            return result;
        }

        std::string output =
            "Refunds found for payment: " + paymentId;

        for (const Refund* refund : refunds)
        {
            if (refund == nullptr)
            {
                continue;
            }

            output +=
                " | id=" + refund->id +
                " | amount=" +
                std::to_string(refund->amount) +
                " | status=" +
                std::to_string(
                    static_cast<int>(refund->status)
                );
        }

        result.setStatus(ToolCallStatus::Succeeded);
        result.setResult(output);

        return result;
    }
}