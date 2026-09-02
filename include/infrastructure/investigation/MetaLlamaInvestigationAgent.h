#pragma once

#include "application/investigation/InvestigationResponseParser.h"
#include "application/investigation/LLMInvestigationAgent.h"
#include "application/investigation/LLMProvider.h"

namespace fincon
{
    class MetaLlamaInvestigationAgent final
        : public LLMInvestigationAgent
    {
    public:
        MetaLlamaInvestigationAgent(
            const LLMProvider& llmProvider,
            const InvestigationResponseParser& responseParser
        );

        InvestigationResponse investigate(
            const InvestigationRequest& request
        ) const override;

    private:
        const LLMProvider& llmProvider_;
        const InvestigationResponseParser& responseParser_;
    };
}