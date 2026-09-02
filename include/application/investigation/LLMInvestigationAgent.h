#pragma once

#include "domain/investigation/InvestigationRequest.h"
#include "domain/investigation/InvestigationResponse.h"

namespace fincon
{
    class LLMInvestigationAgent
    {
    public:
        virtual ~LLMInvestigationAgent() = default;

        virtual InvestigationResponse investigate(
            const InvestigationRequest& request
        ) const = 0;
    };
}