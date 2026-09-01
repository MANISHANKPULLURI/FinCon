#include "infrastructure/reconciliation/rules/MissingRecordRule.h"

#include <chrono>
#include <string>
#include <utility>
#include <vector>

namespace fincon {

std::string MissingRecordRule::id() const {
    return "MISSING_SETTLEMENT_RECONCILIATION";
}

std::vector<ReconciliationFinding>
MissingRecordRule::evaluate(
    const FinancialDataset& dataset) const {

    std::vector<ReconciliationFinding> findings;

    for (const Payment& payment :
         dataset.payments) {

        bool settlementFound = false;

        for (const Settlement& settlement :
             dataset.settlements) {

            for (const std::string& paymentId :
                 settlement.paymentIds) {

                if (paymentId == payment.id) {
                    settlementFound = true;
                    break;
                }
            }

            if (settlementFound) {
                break;
            }
        }

        if (settlementFound) {
            continue;
        }

        ReconciliationFinding finding;

        finding.type =
            FindingType::MissingRecord;

        finding.severity =
            FindingSeverity::Critical;

        finding.entityIds = {
            payment.id
        };

        finding.expectedValue =
            "settlement for payment";

        finding.observedValue =
            "settlement missing";

        finding.financialImpact =
            payment.amount;

        finding.ruleId = id();

        finding.description =
            "Captured payment does not have "
            "a corresponding settlement";

        finding.detectedAt =
            std::chrono::system_clock::now();

        findings.push_back(
            std::move(finding)
        );
    }

    return findings;
}

}