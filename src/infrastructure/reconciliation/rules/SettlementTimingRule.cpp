#include "infrastructure/reconciliation/rules/SettlementTimingRule.h"

#include <chrono>
#include <string>
#include <utility>
#include <vector>

namespace fincon {

std::string SettlementTimingRule::id() const {
    return "SETTLEMENT_TIMING_RECONCILIATION";
}

std::vector<ReconciliationFinding>
SettlementTimingRule::evaluate(
    const FinancialDataset& dataset) const {

    std::vector<ReconciliationFinding> findings;

    constexpr auto expectedDelay =
        std::chrono::hours(1);

    for (const Settlement& settlement :
         dataset.settlements) {

        const BankTransaction* transaction = nullptr;

        for (const BankTransaction& candidate :
             dataset.bankTransactions) {

            if (candidate.settlementId == settlement.id) {
                transaction = &candidate;
                break;
            }
        }

        if (transaction == nullptr) {
            continue;
        }

        const auto actualDelay =
            transaction->postedAt -
            settlement.settledAt;

        if (actualDelay == expectedDelay) {
            continue;
        }

        ReconciliationFinding finding;

        finding.type =
            FindingType::DateMismatch;

        finding.severity =
            FindingSeverity::Medium;

        finding.entityIds = {
            settlement.id,
            transaction->id
        };

        finding.expectedValue =
            "bank posting 1 hour after settlement";

        finding.observedValue =
            "bank posting time differs from expected timing";

        finding.financialImpact = 0;

        finding.ruleId = id();

        finding.description =
            "Bank transaction posting time does not "
            "match expected settlement timing";

        finding.detectedAt =
            std::chrono::system_clock::now();

        findings.push_back(
            std::move(finding)
        );
    }

    return findings;
}

}