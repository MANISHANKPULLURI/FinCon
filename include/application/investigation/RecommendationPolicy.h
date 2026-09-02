#pragma once

#include "domain/incident/Incident.h"
#include "domain/investigation/Investigation.h"
#include "domain/investigation/InvestigationDecision.h"
#include "domain/investigation/InvestigationEvidence.h"
#include "domain/investigation/InvestigationRecommendation.h"

#include <vector>

namespace fincon
{
    class RecommendationPolicy
    {
    public:
        virtual ~RecommendationPolicy() = default;

        virtual InvestigationRecommendation recommend(
            const Incident& incident,
            const Investigation& investigation,
            const InvestigationDecision& decision,
            const std::vector<InvestigationEvidence>& evidence
        ) const = 0;
    };
}