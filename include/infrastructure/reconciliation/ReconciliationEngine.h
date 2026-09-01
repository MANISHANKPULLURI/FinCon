#pragma once

#include "infrastructure/generator/FinancialDataGenerator.h"
#include "infrastructure/reconciliation/ReconciliationFinding.h"
#include "infrastructure/reconciliation/ReconciliationRule.h"

#include <memory>
#include <vector>

namespace fincon {

class ReconciliationEngine {
public:
    ReconciliationEngine();

    void addRule(
        std::unique_ptr<ReconciliationRule> rule
    );

    std::vector<ReconciliationFinding> reconcile(
        const FinancialDataset& dataset
    ) const;

private:
    std::vector<std::unique_ptr<ReconciliationRule>> rules_;
};

}