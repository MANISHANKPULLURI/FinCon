#pragma once

#include "common/Money.h"
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
            const std::vector<InvestigationEvidence>& evidence,
            const std::vector<InvestigationHypothesis>& hypotheses
        ) const = 0;
    };

}