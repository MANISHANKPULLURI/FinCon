#include "infrastructure/reconciliation/rules/RefundReconciliationRule.h"

#include <chrono>
#include <string>
#include <utility>
#include <vector>

namespace fincon {

std::string RefundReconciliationRule::id() const {
    return "REFUND_RECONCILIATION";
}

std::vector<ReconciliationFinding>
RefundReconciliationRule::evaluate(
    const FinancialDataset& dataset) const {

    std::vector<ReconciliationFinding> findings;

    std::size_t counter = 1;

    for (const Payment& payment : dataset.payments) {

        Money totalRefunded = 0;
        std::vector<std::string> refundIds;

        for (const Refund& refund : dataset.refunds) {

            if (refund.paymentId != payment.id) {
                continue;
            }

            totalRefunded += refund.amount;
            refundIds.push_back(refund.id);
        }

        if (totalRefunded <= payment.amount) {
            continue;
        }

        ReconciliationFinding finding;

        finding.id =
            "RF-" + std::to_string(counter++);

        finding.type =
            FindingType::RefundMismatch;

        finding.severity =
            FindingSeverity::High;

        finding.entityIds.push_back(payment.id);

        for (const std::string& refundId : refundIds) {
            finding.entityIds.push_back(refundId);
        }

        finding.expectedValue =
            std::to_string(payment.amount);

        finding.observedValue =
            std::to_string(totalRefunded);

        finding.financialImpact =
            totalRefunded - payment.amount;

        finding.ruleId = id();

        finding.description =
            "Total refunded amount exceeds "
            "the captured payment amount";

        finding.detectedAt =
            std::chrono::system_clock::now();

        findings.push_back(
            std::move(finding)
        );
    }

    return findings;
}

}