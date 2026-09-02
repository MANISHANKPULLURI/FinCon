#pragma once

#include "domain/incident/Incident.h"
#include "domain/investigation/InvestigationDecision.h"
#include "domain/investigation/InvestigationEvidence.h"
#include "domain/investigation/InvestigationHypothesis.h"

#include <vector>

namespace fincon
{
    class DecisionPolicy
    {
    public:
        virtual ~DecisionPolicy() = default;

        virtual InvestigationDecision decide(
            const Incident& incident,
            const std::vector<InvestigationEvidence>& evidence,
            const std::vector<InvestigationHypothesis>& hypotheses,
            const Money& confirmedImpact
        ) const = 0;
    };
}