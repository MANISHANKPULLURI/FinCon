#pragma once

#include "domain/incident/Incident.h"
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
            const Incident& incident,
            const std::vector<InvestigationEvidence>& evidence
        ) const = 0;
    };
}