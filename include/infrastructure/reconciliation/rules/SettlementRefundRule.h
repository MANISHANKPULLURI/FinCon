#pragma once

#include "infrastructure/reconciliation/ReconciliationRule.h"

namespace fincon {

class SettlementRefundRule final
    : public ReconciliationRule {
public:
    std::string id() const override;

    std::vector<ReconciliationFinding> evaluate(
        const FinancialDataset& dataset
    ) const override;
};

}