#pragma once

#include "application/investigation/ImpactCalculator.h"

namespace fincon
{
    class DeterministicImpactCalculator final
        : public ImpactCalculator
    {
    public:
        Money calculate(
            const Incident& incident,
            const std::vector<InvestigationEvidence>& evidence,
            const std::vector<InvestigationHypothesis>& hypotheses
        ) const override;
    };
}