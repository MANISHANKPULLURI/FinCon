#include "infrastructure/investigation/DeterministicInvestigationToolRequestValidator.h"

namespace fincon
{
    bool DeterministicInvestigationToolRequestValidator::validate(
        const InvestigationToolRequest& request,
        std::string& error
    ) const
    {
        if (request.toolName().empty())
        {
            error = "Investigation tool name is empty";
            return false;
        }

        if (request.input().empty())
        {
            error = "Investigation tool input is empty";
            return false;
        }

        if (request.toolName() != "get_payment" &&
            request.toolName() != "get_settlement" &&
            request.toolName() != "get_refunds" &&
            request.toolName() != "get_bank_transactions" &&
            request.toolName() != "get_accounting_entries" &&
            request.toolName() != "get_related_transactions" &&
            request.toolName() != "calculate_difference")
        {
            error =
                "Investigation tool is not supported: " +
                request.toolName();

            return false;
        }

        return true;
    }
}