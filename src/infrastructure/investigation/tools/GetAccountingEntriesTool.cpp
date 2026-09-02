#include "infrastructure/investigation/tools/GetAccountingEntriesTool.h"

#include <string>
#include <vector>

namespace fincon
{
    GetAccountingEntriesTool::GetAccountingEntriesTool(
        const FinancialDataRepository& repository)
        : repository_(repository)
    {
    }

    InvestigationToolCall GetAccountingEntriesTool::execute(
        const InvestigationToolCall& request) const
    {
        InvestigationToolCall result = request;

        result.setStatus(ToolCallStatus::Running);

        const std::string& settlementId =
            request.input();

        const std::vector<const AccountingEntry*> entries =
            repository_.getAccountingEntries(settlementId);

        if (entries.empty())
        {
            result.setStatus(ToolCallStatus::Succeeded);

            result.setResult(
                "No accounting entries found for settlement: " +
                settlementId
            );

            return result;
        }

        std::string output =
            "Accounting entries found for settlement: " +
            settlementId;

        for (const AccountingEntry* entry : entries)
        {
            if (entry == nullptr)
            {
                continue;
            }

            output +=
                " | id=" + entry->id +
                " | reference=" + entry->reference +
                " | amount=" +
                std::to_string(entry->amount) +
                " | type=" +
                std::to_string(
                    static_cast<int>(entry->type)
                );
        }

        result.setStatus(ToolCallStatus::Succeeded);
        result.setResult(output);

        return result;
    }
}