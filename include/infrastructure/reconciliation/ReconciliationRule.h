#pragma once

#include "infrastructure/generator/FinancialDataGenerator.h"
#include "infrastructure/reconciliation/ReconciliationFinding.h"

#include <string>
#include <vector>

namespace fincon {

class ReconciliationRule {
public:
    virtual ~ReconciliationRule() = default;

    virtual std::string id() const = 0;

    virtual std::vector<ReconciliationFinding> evaluate(
        const FinancialDataset& dataset
    ) const = 0;
};

}