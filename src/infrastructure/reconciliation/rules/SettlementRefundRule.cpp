#include "infrastructure/reconciliation/rules/SettlementRefundRule.h"

#include <chrono>
#include <string>
#include <utility>
#include <vector>

namespace fincon {

std::string SettlementRefundRule::id() const {
    return "SETTLEMENT_REFUND_RECONCILIATION";
}

std::vector<ReconciliationFinding>
SettlementRefundRule::evaluate(
    const FinancialDataset& dataset) const {

    std::vector<ReconciliationFinding> findings;

    for (const Settlement& settlement :
         dataset.settlements) {

        Money expectedRefund = 0;
        std::vector<std::string> refundIds;

        for (const std::string& paymentId :
             settlement.paymentIds) {

            for (const Refund& refund :
                 dataset.refunds) {

                if (refund.paymentId != paymentId) {
                    continue;
                }

                expectedRefund += refund.amount;
                refundIds.push_back(refund.id);
            }
        }

        if (expectedRefund == settlement.refundAmount) {
            continue;
        }

        ReconciliationFinding finding;

        finding.type =
            FindingType::RefundMismatch;

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

        for (const std::string& refundId :
             refundIds) {

            finding.entityIds.push_back(
                refundId
            );
        }

        finding.expectedValue =
            std::to_string(expectedRefund);

        finding.observedValue =
            std::to_string(settlement.refundAmount);

        finding.financialImpact =
            expectedRefund >= settlement.refundAmount
                ? expectedRefund - settlement.refundAmount
                : settlement.refundAmount - expectedRefund;

        finding.ruleId = id();

        finding.description =
            "Settlement refund amount does not match "
            "the total of associated refund records";

        finding.detectedAt =
            std::chrono::system_clock::now();

        findings.push_back(
            std::move(finding)
        );
    }

    return findings;
}

}