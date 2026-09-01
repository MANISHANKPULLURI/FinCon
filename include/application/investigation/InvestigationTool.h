#pragma once

#include "domain/investigation/InvestigationToolCall.h"

namespace fincon
{

    class InvestigationTool
    {
    public:
        virtual ~InvestigationTool() = default;

        virtual InvestigationToolCall execute(
            const InvestigationToolCall& request
        ) const = 0;
    };

}