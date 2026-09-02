#pragma once

#include "application/investigation/InvestigationCompletenessEvaluator.h"

namespace fincon
{
    class DeterministicInvestigationCompletenessEvaluator final
        : public InvestigationCompletenessEvaluator
    {
    public:
        InvestigationCompleteness evaluate(
            const Investigation& investigation) const override;
    };
}