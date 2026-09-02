#pragma once

#include "domain/incident/Incident.h"
#include "domain/investigation/InvestigationEvidence.h"
#include "domain/investigation/InvestigationHypothesis.h"

#include <vector>

namespace fincon
{
    class ImpactCalculator
    {
    public:
        virtual ~ImpactCalculator() = default;

    virtual Money calculate(
    const Incident& incident,
    const std::vector<InvestigationEvidence>& evidence,
    const std::vector<InvestigationHypothesis>& hypotheses
) const = 0;
    };
}