#include "infrastructure/investigation/DeterministicRecommendationPolicy.h"

namespace fincon
{
    InvestigationRecommendation
    DeterministicRecommendationPolicy::recommend(
        const Incident&,
        const Investigation& investigation,
        const InvestigationDecision& decision,
        const std::vector<InvestigationEvidence>&) const
    {
        InvestigationRecommendation recommendation(
            "REC-" + investigation.id()
        );

        recommendation.setConfidence(
            decision.confidence()
        );

        recommendation.setFinancialImpact(
            decision.financialImpact()
        );

        for (const std::string& evidenceId :
             decision.evidenceIds())
        {
            recommendation.addEvidence(
                evidenceId
            );
        }

        switch (decision.type())
        {
        case DecisionType::AutoResolve:
            recommendation.setAction(
                RecommendationAction::MarkResolved
            );
            recommendation.setRationale(
                "Exception is strongly supported and can be safely resolved."
            );
            break;

        case DecisionType::HumanReview:
            recommendation.setAction(
                RecommendationAction::EscalateToFinance
            );
            recommendation.setRationale(
                "Financial exposure is confirmed and requires finance approval."
            );
            break;

        case DecisionType::RequestMoreEvidence:
            recommendation.setAction(
                RecommendationAction::RequestEvidence
            );
            recommendation.setRationale(
                "Additional evidence is required before financial resolution."
            );
            break;

        case DecisionType::Unresolved:
            recommendation.setAction(
                RecommendationAction::EscalateToFinance
            );
            recommendation.setRationale(
                "The exception remains unresolved and requires finance investigation."
            );
            break;
        }

        return recommendation;
    }
}
