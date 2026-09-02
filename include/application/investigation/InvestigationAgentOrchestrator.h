#pragma once

#include "application/investigation/InvestigationResponseValidator.h"
#include "application/investigation/InvestigationToolRegistry.h"
#include "application/investigation/InvestigationToolRequestValidator.h"
#include "application/investigation/LLMInvestigationAgent.h"
#include "domain/investigation/InvestigationRequest.h"
#include "domain/investigation/InvestigationResponse.h"

#include <cstddef>

namespace fincon
{
    class InvestigationAgentOrchestrator
    {
    public:
        InvestigationAgentOrchestrator(
            const LLMInvestigationAgent& agent,
            const InvestigationToolRegistry& toolRegistry,
            const InvestigationToolRequestValidator& toolRequestValidator,
            const InvestigationResponseValidator& responseValidator,
            std::size_t maxIterations = 5
        );

        InvestigationResponse investigate(
            InvestigationRequest request
        ) const;

    private:
        const LLMInvestigationAgent& agent_;
        const InvestigationToolRegistry& toolRegistry_;
        const InvestigationToolRequestValidator& toolRequestValidator_;
        const InvestigationResponseValidator& responseValidator_;
        std::size_t maxIterations_;
    };
}