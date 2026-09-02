#pragma once

#include "application/investigation/InvestigationResponseValidator.h"

namespace fincon
{
    class DeterministicInvestigationResponseValidator final
        : public InvestigationResponseValidator
    {
    public:
        ~DeterministicInvestigationResponseValidator() override = default;

        bool validate(
            const InvestigationResponse& response,
            const InvestigationRequest& request,
            std::string& error
        ) const override;
    };
}