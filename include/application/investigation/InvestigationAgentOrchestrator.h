#pragma once

#include "application/investigation/InvestigationToolRegistry.h"
#include "application/investigation/LLMInvestigationAgent.h"
#include "domain/investigation/InvestigationResponse.h"
#include "domain/investigation/InvestigationRequest.h"

#include <cstddef>

namespace fincon
{
    class InvestigationAgentOrchestrator
    {
    public:
        explicit InvestigationAgentOrchestrator(
            const LLMInvestigationAgent& agent,
            const InvestigationToolRegistry& toolRegistry,
            std::size_t maxIterations = 5
        );

        InvestigationResponse investigate(
            InvestigationRequest request
        ) const;

    private:
        const LLMInvestigationAgent& agent_;
        const InvestigationToolRegistry& toolRegistry_;
        std::size_t maxIterations_;
    };
}