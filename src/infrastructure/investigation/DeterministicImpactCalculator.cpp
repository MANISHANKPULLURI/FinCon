#include "infrastructure/investigation/DeterministicImpactCalculator.h"

namespace fincon
{
    Money DeterministicImpactCalculator::calculate(
        const Incident& incident,
        const std::vector<InvestigationEvidence>&,
        const std::vector<InvestigationHypothesis>& hypotheses) const
    {
        for (const InvestigationHypothesis& hypothesis : hypotheses)
        {
            if (hypothesis.status() == HypothesisStatus::Supported)
            {
                return hypothesis.estimatedImpact();
            }
        }

        return incident.financialImpact();
    }
}