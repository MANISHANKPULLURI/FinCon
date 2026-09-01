#pragma once

#include "domain/investigation/InvestigationEvidence.h"
#include "domain/investigation/InvestigationHypothesis.h"

#include <vector>

namespace fincon
{

    class HypothesisEngine
    {
    public:
        virtual ~HypothesisEngine() = default;

        virtual std::vector<InvestigationHypothesis> generate(
            const std::vector<InvestigationEvidence>& evidence
        ) const = 0;
    };

}