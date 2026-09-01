#pragma once

#include "application/investigation/InvestigationContext.h"
#include "domain/investigation/InvestigationToolCall.h"

#include <vector>

namespace fincon
{

    class InvestigationPlanner
    {
    public:
        virtual ~InvestigationPlanner() = default;

        virtual std::vector<InvestigationToolCall> plan(
            const InvestigationContext& context
        ) const = 0;
    };

}