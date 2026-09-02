#include "application/investigation/DeterministicInvestigationPlanner.h"

#include <string>
#include <vector>

namespace fincon
{
    namespace
    {
        void addToolCall(
            std::vector<InvestigationToolCall>& calls,
            const std::string& toolName,
            const std::string& input)
        {
            if (input.empty())
            {
                return;
            }

            InvestigationToolCall call(
                "TC-" +
                std::to_string(calls.size() + 1)
            );

            call.setToolName(toolName);
            call.setInput(input);
            call.setStatus(ToolCallStatus::Requested);

            calls.push_back(std::move(call));
        }

        std::string findEntity(
            const Incident& incident,
            const std::string& prefix)
        {
            for (const std::string& entityId : incident.entityIds())
            {
                if (entityId.rfind(prefix, 0) == 0)
                {
                    return entityId;
                }
            }

            return {};
        }

        std::string findBaseSettlement(
            const Incident& incident)
        {
            for (const std::string& entityId : incident.entityIds())
            {
                if (entityId.rfind("S-", 0) == 0 &&
                    entityId.find("-DUP") == std::string::npos)
                {
                    return entityId;
                }
            }

            return {};
        }
    }

    std::vector<InvestigationToolCall>
    DeterministicInvestigationPlanner::plan(
        const InvestigationContext& context) const
    {
        std::vector<InvestigationToolCall> calls;

        if (context.incident == nullptr)
        {
            return calls;
        }

        const Incident& incident = *context.incident;

        const std::string paymentId =
            findEntity(incident, "P-");

        const std::string settlementId =
            findBaseSettlement(incident);

        const std::string refundId =
            findEntity(incident, "R-");

        const std::string bankTransactionId =
            findEntity(incident, "B-");

        const std::string accountingEntryId =
            findEntity(incident, "A-");

        switch (incident.type())
        {
        case IncidentType::DuplicateRecord:

            addToolCall(
                calls,
                "get_payment",
                paymentId
            );

            addToolCall(
                calls,
                "get_settlement",
                settlementId
            );

            addToolCall(
                calls,
                "get_related_transactions",
                settlementId
            );

            break;

        case IncidentType::MissingRecord:

            addToolCall(
                calls,
                "get_payment",
                paymentId
            );

            addToolCall(
                calls,
                "get_settlement",
                settlementId
            );

            addToolCall(
                calls,
                "get_related_transactions",
                paymentId
            );

            break;

        case IncidentType::RefundIssue:

            addToolCall(
                calls,
                "get_payment",
                paymentId
            );

            addToolCall(
                calls,
                "get_refunds",
                paymentId
            );

            addToolCall(
                calls,
                "get_settlement",
                settlementId
            );

            break;

        case IncidentType::FeeIssue:

            addToolCall(
                calls,
                "get_payment",
                paymentId
            );

            addToolCall(
                calls,
                "get_settlement",
                settlementId
            );

            addToolCall(
                calls,
                "get_related_transactions",
                paymentId
            );

            addToolCall(
                calls,
                "get_accounting_entries",
                settlementId
            );

            break;

        case IncidentType::TimingIssue:

            addToolCall(
                calls,
                "get_settlement",
                settlementId
            );

            addToolCall(
                calls,
                "get_bank_transactions",
                settlementId
            );

            break;

        case IncidentType::SettlementIssue:

            addToolCall(
                calls,
                "get_payment",
                paymentId
            );

            addToolCall(
                calls,
                "get_settlement",
                settlementId
            );

            addToolCall(
                calls,
                "calculate_difference",
                settlementId
            );

            break;

        case IncidentType::BankIssue:

            addToolCall(
                calls,
                "get_settlement",
                settlementId
            );

            addToolCall(
                calls,
                "get_bank_transactions",
                settlementId
            );

            addToolCall(
                calls,
                "calculate_difference",
                settlementId
            );

            break;

        case IncidentType::AccountingIssue:

            addToolCall(
                calls,
                "get_settlement",
                settlementId
            );

            addToolCall(
                calls,
                "get_accounting_entries",
                settlementId
            );

            addToolCall(
                calls,
                "calculate_difference",
                settlementId
            );

            break;

        default:
            break;
        }

        return calls;
    }
}