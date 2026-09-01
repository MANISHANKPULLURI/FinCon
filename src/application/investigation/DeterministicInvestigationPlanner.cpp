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
            InvestigationToolCall call(
                "TC-" +
                std::to_string(calls.size() + 1)
            );

            call.setToolName(toolName);
            call.setInput(input);
            call.setStatus(ToolCallStatus::Requested);

            calls.push_back(std::move(call));
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

        switch (incident.type())
        {
        case IncidentType::DuplicateRecord:

            addToolCall(
                calls,
                "get_payment",
                "Inspect payment related to duplicate settlement"
            );

            addToolCall(
                calls,
                "get_settlement",
                "Inspect all settlements related to duplicate record"
            );

            addToolCall(
                calls,
                "get_related_transactions",
                "Find transactions related to duplicate settlement"
            );

            break;

        case IncidentType::MissingRecord:

            addToolCall(
                calls,
                "get_payment",
                "Inspect payment associated with missing settlement"
            );

            addToolCall(
                calls,
                "get_settlement",
                "Search for settlement associated with payment"
            );

            addToolCall(
                calls,
                "get_related_transactions",
                "Find related transactions for missing record"
            );

            break;

        case IncidentType::RefundIssue:

            addToolCall(
                calls,
                "get_payment",
                "Inspect payment associated with refund mismatch"
            );

            addToolCall(
                calls,
                "get_refunds",
                "Inspect refunds associated with payment"
            );

            addToolCall(
                calls,
                "get_settlement",
                "Inspect settlement containing payment"
            );

            break;

        case IncidentType::FeeIssue:

            addToolCall(
                calls,
                "get_payment",
                "Inspect payment used for settlement calculation"
            );

            addToolCall(
                calls,
                "get_settlement",
                "Inspect settlement and applied fees"
            );

            addToolCall(
                calls,
                "get_related_transactions",
                "Inspect related fee transactions"
            );

            addToolCall(
                calls,
                "get_accounting_entries",
                "Inspect accounting entries for settlement"
            );

            break;

        case IncidentType::TimingIssue:

            addToolCall(
                calls,
                "get_settlement",
                "Inspect settlement processing date"
            );

            addToolCall(
                calls,
                "get_bank_transactions",
                "Inspect corresponding bank transaction date"
            );

            break;

        case IncidentType::SettlementIssue:

            addToolCall(
                calls,
                "get_payment",
                "Inspect payment contributing to settlement"
            );

            addToolCall(
                calls,
                "get_settlement",
                "Inspect settlement calculation"
            );

            addToolCall(
                calls,
                "calculate_difference",
                "Calculate settlement amount difference"
            );

            break;

        case IncidentType::BankIssue:

            addToolCall(
                calls,
                "get_settlement",
                "Inspect settlement associated with bank discrepancy"
            );

            addToolCall(
                calls,
                "get_bank_transactions",
                "Inspect corresponding bank transactions"
            );

            addToolCall(
                calls,
                "calculate_difference",
                "Calculate settlement versus bank difference"
            );

            break;

        case IncidentType::AccountingIssue:

            addToolCall(
                calls,
                "get_settlement",
                "Inspect settlement associated with accounting discrepancy"
            );

            addToolCall(
                calls,
                "get_accounting_entries",
                "Inspect accounting entries for settlement"
            );

            addToolCall(
                calls,
                "calculate_difference",
                "Calculate settlement versus accounting difference"
            );

            break;

        default:
            break;
        }

        return calls;
    }

}