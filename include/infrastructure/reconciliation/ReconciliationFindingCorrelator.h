#pragma once

#include "infrastructure/reconciliation/ReconciliationFinding.h"

#include <vector>

namespace fincon {

class ReconciliationFindingCorrelator final {
public:
    std::vector<ReconciliationFinding> correlate(
        const std::vector<ReconciliationFinding>& findings
    ) const;
};

}