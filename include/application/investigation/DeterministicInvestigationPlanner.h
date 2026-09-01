#pragma once

#include "application/investigation/InvestigationPlanner.h"

namespace fincon
{

    class DeterministicInvestigationPlanner final
        : public InvestigationPlanner
    {
    public:
        std::vector<InvestigationToolCall> plan(
            const InvestigationContext& context
        ) const override;
    };

}