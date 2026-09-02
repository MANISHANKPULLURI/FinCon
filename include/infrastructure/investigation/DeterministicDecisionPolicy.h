#pragma once

#include "application/investigation/DecisionPolicy.h"

namespace fincon
{
    class DeterministicDecisionPolicy final
        : public DecisionPolicy
    {
    public:
        InvestigationDecision decide(
            const Incident& incident,
            const std::vector<InvestigationEvidence>& evidence,
            const std::vector<InvestigationHypothesis>& hypotheses,
            const Money& confirmedImpact
        ) const override;
    };
}