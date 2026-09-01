#include "infrastructure/reconciliation/rules/SettlementBankRule.h"

#include <chrono>
#include <string>
#include <utility>
#include <vector>

namespace fincon {

std::string SettlementBankRule::id() const {
    return "SETTLEMENT_BANK_RECONCILIATION";
}

std::vector<ReconciliationFinding>
SettlementBankRule::evaluate(
    const FinancialDataset& dataset) const {

    std::vector<ReconciliationFinding> findings;

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
            ReconciliationFinding finding;

            finding.type =
                FindingType::MissingRecord;

            finding.severity =
                FindingSeverity::Critical;

            finding.entityIds.push_back(
                settlement.id
            );

            finding.expectedValue =
                "bank transaction";

            finding.observedValue =
                "missing";

            finding.financialImpact =
                settlement.netAmount;

            finding.ruleId = id();

            finding.description =
                "Settlement does not have "
                "a corresponding bank transaction";

            finding.detectedAt =
                std::chrono::system_clock::now();

            findings.push_back(
                std::move(finding)
            );

            continue;
        }

        if (transaction->amount == settlement.netAmount) {
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

        finding.entityIds.push_back(
            transaction->id
        );

        finding.expectedValue =
            std::to_string(settlement.netAmount);

        finding.observedValue =
            std::to_string(transaction->amount);

        finding.financialImpact =
            settlement.netAmount >= transaction->amount
                ? settlement.netAmount - transaction->amount
                : transaction->amount - settlement.netAmount;

        finding.ruleId = id();

        finding.description =
            "Bank transaction amount does not match "
            "settlement net amount";

        finding.detectedAt =
            std::chrono::system_clock::now();

        findings.push_back(
            std::move(finding)
        );
    }

    return findings;
}

}