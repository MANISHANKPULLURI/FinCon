#include "infrastructure/investigation/DeterministicDecisionPolicy.h"

#include <string>

namespace fincon
{
    InvestigationDecision
    DeterministicDecisionPolicy::decide(
        const Incident& incident,
        const std::vector<InvestigationEvidence>& evidence,
        const std::vector<InvestigationHypothesis>& hypotheses,
        const Money& confirmedImpact) const
    {
        InvestigationDecision decision(
            "DEC-1"
        );

        decision.setFinancialImpact(
            confirmedImpact
        );

        if (evidence.empty())
        {
            decision.setType(
                DecisionType::RequestMoreEvidence
            );

            decision.setConfidence(
                DecisionConfidence::Low
            );

            decision.setRationale(
                "Insufficient evidence to determine the exception."
            );

            return decision;
        }

        const InvestigationHypothesis* strongestHypothesis =
            nullptr;

        for (const InvestigationHypothesis& hypothesis :
             hypotheses)
        {
            if (hypothesis.status() != HypothesisStatus::Supported)
            {
                continue;
            }

            if (strongestHypothesis == nullptr ||
                hypothesis.confidenceScore() >
                    strongestHypothesis->confidenceScore())
            {
                strongestHypothesis = &hypothesis;
            }
        }

        if (strongestHypothesis == nullptr)
        {
            decision.setType(
                DecisionType::Unresolved
            );

            decision.setConfidence(
                DecisionConfidence::Low
            );

            decision.setRationale(
                "No supported hypothesis was established from the available evidence."
            );

            return decision;
        }

        for (const std::string& evidenceId :
             strongestHypothesis->evidenceIds())
        {
            decision.addEvidence(
                evidenceId
            );
        }

        const int confidenceScore =
            strongestHypothesis->confidenceScore();

        if (confidenceScore >= 90)
        {
            decision.setConfidence(
                DecisionConfidence::High
            );
        }
        else if (confidenceScore >= 70)
        {
            decision.setConfidence(
                DecisionConfidence::Medium
            );
        }
        else
        {
            decision.setConfidence(
                DecisionConfidence::Low
            );
        }

        if (incident.type() == IncidentType::MissingRecord)
        {
            decision.setType(
                DecisionType::RequestMoreEvidence
            );

            decision.setRationale(
                "The investigated record is missing and additional evidence is required before resolution."
            );

            return decision;
        }

        if (incident.type() == IncidentType::TimingIssue &&
            confirmedImpact == 0 &&
            confidenceScore >= 90)
        {
            decision.setType(
                DecisionType::AutoResolve
            );

            decision.setRationale(
                "The timing exception is strongly supported and has no confirmed financial exposure."
            );

            return decision;
        }

        if (confidenceScore < 90)
        {
            decision.setType(
                DecisionType::RequestMoreEvidence
            );

            decision.setRationale(
                "The available evidence supports the hypothesis but confidence is insufficient for resolution."
            );

            return decision;
        }

        if (confirmedImpact == 0)
        {
            decision.setType(
                DecisionType::AutoResolve
            );

            decision.setRationale(
                "The exception is strongly supported and has no confirmed financial exposure."
            );

            return decision;
        }

        decision.setType(
            DecisionType::HumanReview
        );

        decision.setRationale(
            "The exception is strongly supported but requires human review because financial exposure is confirmed."
        );

        return decision;
    }
}