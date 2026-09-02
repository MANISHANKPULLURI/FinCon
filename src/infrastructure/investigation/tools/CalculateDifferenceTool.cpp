#include "infrastructure/investigation/tools/CalculateDifferenceTool.h"

#include <cstdlib>
#include <string>
#include <vector>

namespace fincon
{
    CalculateDifferenceTool::CalculateDifferenceTool(
        const FinancialDataRepository& repository)
        : repository_(repository)
    {
    }

    InvestigationToolCall CalculateDifferenceTool::execute(
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

        const std::vector<const BankTransaction*> bankTransactions =
            repository_.getBankTransactions(settlementId);

        const std::vector<const AccountingEntry*> accountingEntries =
            repository_.getAccountingEntries(settlementId);

        if (bankTransactions.empty() &&
            accountingEntries.empty())
        {
            result.setStatus(ToolCallStatus::Succeeded);

            result.setResult(
                "No comparison records found for settlement: " +
                settlementId
            );

            return result;
        }

        std::string output =
            "Difference analysis for settlement: " +
            settlementId;

        for (const BankTransaction* transaction : bankTransactions)
        {
            if (transaction == nullptr)
            {
                continue;
            }

            const Money difference =
                transaction->amount - settlement->netAmount;

            output +=
                " | bankTransaction=" +
                transaction->id +
                " | settlementNet=" +
                std::to_string(settlement->netAmount) +
                " | bankAmount=" +
                std::to_string(transaction->amount) +
                " | difference=" +
                std::to_string(difference);
        }

        for (const AccountingEntry* entry : accountingEntries)
        {
            if (entry == nullptr)
            {
                continue;
            }

            const Money difference =
                entry->amount - settlement->netAmount;

            output +=
                " | accountingEntry=" +
                entry->id +
                " | settlementNet=" +
                std::to_string(settlement->netAmount) +
                " | accountingAmount=" +
                std::to_string(entry->amount) +
                " | difference=" +
                std::to_string(difference);
        }

        result.setStatus(ToolCallStatus::Succeeded);
        result.setResult(output);

        return result;
    }
}