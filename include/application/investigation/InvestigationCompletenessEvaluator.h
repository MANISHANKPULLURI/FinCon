#pragma once

#include "domain/investigation/Investigation.h"

namespace fincon
{
    enum class InvestigationCompleteness
    {
        Complete,
        Incomplete
    };

    class InvestigationCompletenessEvaluator
    {
    public:
        virtual ~InvestigationCompletenessEvaluator() = default;

        virtual InvestigationCompleteness evaluate(
            const Investigation& investigation) const = 0;
    };
}