#include "infrastructure/reconciliation/rules/SettlementCalculationRule.h"

#include <chrono>
#include <string>
#include <utility>
#include <vector>

namespace fincon {

std::string SettlementCalculationRule::id() const {
    return "SETTLEMENT_CALCULATION";
}

std::vector<ReconciliationFinding>
SettlementCalculationRule::evaluate(
    const FinancialDataset& dataset) const {

    std::vector<ReconciliationFinding> findings;

    for (const Settlement& settlement :
         dataset.settlements) {

        const Money expected =
            settlement.grossAmount
            - settlement.refundAmount
            - settlement.feeAmount;

        if (expected == settlement.netAmount) {
            continue;
        }

        ReconciliationFinding finding;

        finding.type =
            FindingType::AmountMismatch;

        finding.severity =
            FindingSeverity::High;

        finding.entityIds.push_back(
            settlement.id
        );

        finding.expectedValue =
            std::to_string(expected);

        finding.observedValue =
            std::to_string(settlement.netAmount);

        finding.financialImpact =
            expected >= settlement.netAmount
                ? expected - settlement.netAmount
                : settlement.netAmount - expected;

        finding.ruleId = id();

        finding.description =
            "Settlement net amount does not match "
            "gross amount minus refunds and fees";

        finding.detectedAt =
            std::chrono::system_clock::now();

        findings.push_back(
            std::move(finding)
        );
    }

    return findings;
}

}