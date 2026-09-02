#pragma once

#include "application/investigation/InvestigationToolRequestValidator.h"

namespace fincon
{
    class DeterministicInvestigationToolRequestValidator final
        : public InvestigationToolRequestValidator
    {
    public:
        ~DeterministicInvestigationToolRequestValidator() override = default;

        bool validate(
            const InvestigationToolRequest& request,
            std::string& error
        ) const override;
    };
}