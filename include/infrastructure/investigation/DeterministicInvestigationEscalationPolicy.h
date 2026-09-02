#pragma once

#include "application/investigation/InvestigationEscalationPolicy.h"

namespace fincon
{
    class DeterministicInvestigationEscalationPolicy final
        : public InvestigationEscalationPolicy
    {
    public:
        EscalationDecision evaluate(
            const Incident& incident,
            const Investigation& investigation) const override;
    };
}