#pragma once

#include "domain/incident/FindingCorrelation.h"
#include "infrastructure/reconciliation/ReconciliationFinding.h"

#include <vector>

namespace fincon
{

    class ReconciliationFindingCorrelator final
    {
    public:

        std::vector<FindingCorrelation> correlate(
            const std::vector<ReconciliationFinding>& findings
        ) const;
    };

}