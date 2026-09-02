#pragma once

#include "application/investigation/HypothesisEngine.h"

namespace fincon
{
    class DeterministicHypothesisEngine final
        : public HypothesisEngine
    {
    public:
        std::vector<InvestigationHypothesis> generate(
            const Incident& incident,
            const std::vector<InvestigationEvidence>& evidence
        ) const override;
    };
}