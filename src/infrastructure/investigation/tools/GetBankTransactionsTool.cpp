#include "infrastructure/investigation/tools/GetBankTransactionsTool.h"

#include <string>
#include <vector>

namespace fincon
{
    GetBankTransactionsTool::GetBankTransactionsTool(
        const FinancialDataRepository& repository)
        : repository_(repository)
    {
    }

    InvestigationToolCall GetBankTransactionsTool::execute(
        const InvestigationToolCall& request) const
    {
        InvestigationToolCall result = request;

        result.setStatus(ToolCallStatus::Running);

        const std::string& settlementId =
            request.input();

        const std::vector<const BankTransaction*> transactions =
            repository_.getBankTransactions(settlementId);

        if (transactions.empty())
        {
            result.setStatus(ToolCallStatus::Succeeded);

            result.setResult(
                "No bank transactions found for settlement: " +
                settlementId
            );

            return result;
        }

        std::string output =
            "Bank transactions found for settlement: " +
            settlementId;

        for (const BankTransaction* transaction : transactions)
        {
            if (transaction == nullptr)
            {
                continue;
            }

            output +=
                " | id=" + transaction->id +
                " | amount=" +
                std::to_string(transaction->amount) +
                " | type=" +
                std::to_string(
                    static_cast<int>(transaction->type)
                );
        }

        result.setStatus(ToolCallStatus::Succeeded);
        result.setResult(output);

        return result;
    }
}