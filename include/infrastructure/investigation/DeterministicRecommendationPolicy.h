#pragma once

#include "application/investigation/RecommendationPolicy.h"

namespace fincon
{
    class DeterministicRecommendationPolicy final
        : public RecommendationPolicy
    {
    public:
        InvestigationRecommendation recommend(
            const Incident& incident,
            const Investigation& investigation,
            const InvestigationDecision& decision,
            const std::vector<InvestigationEvidence>& evidence
        ) const override;
    };
}