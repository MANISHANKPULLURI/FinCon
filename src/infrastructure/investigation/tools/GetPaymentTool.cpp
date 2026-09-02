#include "infrastructure/investigation/tools/GetPaymentTool.h"

#include <string>

namespace fincon
{
    GetPaymentTool::GetPaymentTool(
        const FinancialDataRepository& repository)
        : repository_(repository)
    {
    }

    InvestigationToolCall GetPaymentTool::execute(
        const InvestigationToolCall& request) const
    {
        InvestigationToolCall result = request;

        result.setStatus(ToolCallStatus::Running);

        const std::string& paymentId = request.input();

        const Payment* payment =
            repository_.getPayment(paymentId);

        if (payment == nullptr)
        {
            result.setStatus(ToolCallStatus::Failed);

            result.setResult(
                "Payment not found: " + paymentId
            );

            return result;
        }

        result.setStatus(ToolCallStatus::Succeeded);

        result.setResult(
            "Payment found"
            " | id=" + payment->id +
            " | merchantId=" + payment->merchantId +
            " | orderId=" + payment->orderId +
            " | amount=" + std::to_string(payment->amount) +
            " | status=" +
            std::to_string(
                static_cast<int>(payment->status)
            )
        );

        return result;
    }
}