#include "infrastructure/reconciliation/rules/SettlementFeeRule.h"

#include <chrono>
#include <string>
#include <utility>
#include <vector>

namespace fincon {

std::string SettlementFeeRule::id() const {
    return "SETTLEMENT_FEE_RECONCILIATION";
}

std::vector<ReconciliationFinding>
SettlementFeeRule::evaluate(
    const FinancialDataset& dataset) const {

    std::vector<ReconciliationFinding> findings;

    for (const Settlement& settlement :
         dataset.settlements) {

        Money expectedFee = 0;
        std::vector<std::string> feeIds;

        for (const std::string& paymentId :
             settlement.paymentIds) {

            for (const Fee& fee : dataset.fees) {

                if (fee.paymentId != paymentId) {
                    continue;
                }

                expectedFee += fee.amount;
                feeIds.push_back(fee.id);
            }
        }

        if (expectedFee == settlement.feeAmount) {
            continue;
        }

        ReconciliationFinding finding;

        finding.type =
            FindingType::FeeDiscrepancy;

        finding.severity =
            FindingSeverity::High;

        finding.entityIds.push_back(
            settlement.id
        );

        for (const std::string& paymentId :
             settlement.paymentIds) {

            finding.entityIds.push_back(
                paymentId
            );
        }

        for (const std::string& feeId : feeIds) {
            finding.entityIds.push_back(feeId);
        }

        finding.expectedValue =
            std::to_string(expectedFee);

        finding.observedValue =
            std::to_string(settlement.feeAmount);

        finding.financialImpact =
            expectedFee >= settlement.feeAmount
                ? expectedFee - settlement.feeAmount
                : settlement.feeAmount - expectedFee;

        finding.ruleId = id();

        finding.description =
            "Settlement fee amount does not match "
            "the total of associated fee records";

        finding.detectedAt =
            std::chrono::system_clock::now();

        findings.push_back(
            std::move(finding)
        );
    }

    return findings;
}

}