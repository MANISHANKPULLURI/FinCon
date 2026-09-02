#include "infrastructure/investigation/tools/GetSettlementTool.h"

#include <string>

namespace fincon
{
    GetSettlementTool::GetSettlementTool(
        const FinancialDataRepository& repository)
        : repository_(repository)
    {
    }

    InvestigationToolCall GetSettlementTool::execute(
        const InvestigationToolCall& request) const
    {
        InvestigationToolCall result = request;

        result.setStatus(ToolCallStatus::Running);

        const std::string& settlementId =
            request.input();

        const Settlement* settlement =
            repository_.getSettlement(settlementId);

        if (settlement == nullptr)
        {
            result.setStatus(ToolCallStatus::Failed);

            result.setResult(
                "Settlement not found: " + settlementId
            );

            return result;
        }

        result.setStatus(ToolCallStatus::Succeeded);

        result.setResult(
            "Settlement found"
            " | id=" + settlement->id +
            " | merchantId=" + settlement->merchantId +
            " | grossAmount=" +
            std::to_string(settlement->grossAmount) +
            " | refundAmount=" +
            std::to_string(settlement->refundAmount) +
            " | feeAmount=" +
            std::to_string(settlement->feeAmount) +
            " | netAmount=" +
            std::to_string(settlement->netAmount) +
            " | status=" +
            std::to_string(
                static_cast<int>(settlement->status)
            )
        );

        return result;
    }
}