#include "infrastructure/investigation/DeterministicInvestigationEscalationPolicy.h"

namespace fincon
{
    EscalationDecision
    DeterministicInvestigationEscalationPolicy::evaluate(
        const Incident&,
        const Investigation& investigation) const
    {
        if (investigation.status() != InvestigationStatus::Completed)
            return EscalationDecision::EscalateToLLM;

        if (investigation.outcome() == InvestigationOutcome::Unknown)
            return EscalationDecision::EscalateToLLM;

        if (investigation.outcome() == InvestigationOutcome::Unresolved)
            return EscalationDecision::EscalateToLLM;

        if (investigation.outcome() == InvestigationOutcome::RequestMoreEvidence)
            return EscalationDecision::EscalateToLLM;

        if (investigation.confidence() == ConfidenceLevel::Low)
            return EscalationDecision::EscalateToLLM;

        if (investigation.evidenceIds().empty())
            return EscalationDecision::EscalateToLLM;

        return EscalationDecision::ContinueDeterministic;
    }
}