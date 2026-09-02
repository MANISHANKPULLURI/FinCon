#include "infrastructure/investigation/DeterministicInvestigationCompletenessEvaluator.h"

namespace fincon
{
    InvestigationCompleteness
    DeterministicInvestigationCompletenessEvaluator::evaluate(
        const Investigation& investigation) const
    {
        if (investigation.status() != InvestigationStatus::Completed)
            return InvestigationCompleteness::Incomplete;

        if (investigation.evidenceIds().empty())
            return InvestigationCompleteness::Incomplete;

        if (investigation.toolCallIds().empty())
            return InvestigationCompleteness::Incomplete;

        if (investigation.outcome() == InvestigationOutcome::Unknown)
            return InvestigationCompleteness::Incomplete;

        if (investigation.confidence() == ConfidenceLevel::Low)
            return InvestigationCompleteness::Incomplete;

        if (investigation.outcome() == InvestigationOutcome::Unresolved)
            return InvestigationCompleteness::Incomplete;

        if (investigation.outcome() == InvestigationOutcome::RequestMoreEvidence)
            return InvestigationCompleteness::Incomplete;

        return InvestigationCompleteness::Complete;
    }
}