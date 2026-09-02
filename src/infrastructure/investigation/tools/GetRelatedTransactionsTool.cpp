#include "infrastructure/investigation/tools/GetRelatedTransactionsTool.h"

#include <string>
#include <vector>

namespace fincon
{
    GetRelatedTransactionsTool::GetRelatedTransactionsTool(
        const FinancialDataRepository& repository)
        : repository_(repository)
    {
    }

    InvestigationToolCall GetRelatedTransactionsTool::execute(
        const InvestigationToolCall& request) const
    {
        InvestigationToolCall result = request;

        result.setStatus(ToolCallStatus::Running);

        const std::string& entityId =
            request.input();

        std::string output;

        if (entityId.rfind("P-", 0) == 0)
        {
            const std::vector<const Refund*> refunds =
                repository_.getRefunds(entityId);

            const std::vector<const Fee*> fees =
                repository_.getFees(entityId);

            output =
                "Related transactions for payment: " +
                entityId;

            for (const Refund* refund : refunds)
            {
                if (refund == nullptr)
                {
                    continue;
                }

                output +=
                    " | refundId=" + refund->id +
                    " | refundAmount=" +
                    std::to_string(refund->amount);
            }

            for (const Fee* fee : fees)
            {
                if (fee == nullptr)
                {
                    continue;
                }

                output +=
                    " | feeId=" + fee->id +
                    " | feeAmount=" +
                    std::to_string(fee->amount);
            }

            result.setStatus(ToolCallStatus::Succeeded);
            result.setResult(output);

            return result;
        }

        if (entityId.rfind("S-", 0) == 0)
        {
            const std::vector<const BankTransaction*> transactions =
                repository_.getBankTransactions(entityId);

            const std::vector<const AccountingEntry*> entries =
                repository_.getAccountingEntries(entityId);

            output =
                "Related transactions for settlement: " +
                entityId;

            for (const BankTransaction* transaction : transactions)
            {
                if (transaction == nullptr)
                {
                    continue;
                }

                output +=
                    " | bankTransactionId=" +
                    transaction->id +
                    " | bankAmount=" +
                    std::to_string(transaction->amount);
            }

            for (const AccountingEntry* entry : entries)
            {
                if (entry == nullptr)
                {
                    continue;
                }

                output +=
                    " | accountingEntryId=" +
                    entry->id +
                    " | accountingAmount=" +
                    std::to_string(entry->amount);
            }

            result.setStatus(ToolCallStatus::Succeeded);
            result.setResult(output);

            return result;
        }

        result.setStatus(ToolCallStatus::Failed);

        result.setResult(
            "Unsupported entity for related transaction lookup: " +
            entityId
        );

        return result;
    }
}