#include "infrastructure/reconciliation/rules/SettlementAccountingRule.h"

#include <chrono>
#include <string>
#include <utility>
#include <vector>

namespace fincon {

std::string SettlementAccountingRule::id() const
{
    return "SETTLEMENT_ACCOUNTING_RECONCILIATION";
}

std::vector<ReconciliationFinding>
SettlementAccountingRule::evaluate(
    const FinancialDataset& dataset) const
{
    std::vector<ReconciliationFinding> findings;

    for (const Settlement& settlement :
         dataset.settlements)
    {
        const AccountingEntry* matchedEntry = nullptr;

        for (const AccountingEntry& entry :
             dataset.accountingEntries)
        {
            if (entry.reference == settlement.id)
            {
                matchedEntry = &entry;
                break;
            }
        }

        if (matchedEntry == nullptr)
        {
            ReconciliationFinding finding;

            finding.type = FindingType::MissingRecord;
            finding.severity = FindingSeverity::High;

            finding.entityIds = {
                settlement.id
            };

            finding.expectedValue =
                "accounting entry for settlement";

            finding.observedValue =
                "accounting entry missing";

            finding.financialImpact =
                settlement.netAmount;

            finding.ruleId = id();

            finding.description =
                "Settlement does not have a corresponding accounting entry";

            finding.detectedAt =
                std::chrono::system_clock::now();

            findings.push_back(std::move(finding));

            continue;
        }

        if (matchedEntry->merchantId !=
            settlement.merchantId)
        {
            ReconciliationFinding finding;

            finding.type = FindingType::AmountMismatch;
            finding.severity = FindingSeverity::Critical;

            finding.entityIds = {
                settlement.id,
                matchedEntry->id
            };

            finding.expectedValue =
                "matching merchant";

            finding.observedValue =
                "accounting entry belongs to different merchant";

            finding.financialImpact =
                settlement.netAmount;

            finding.ruleId = id();

            finding.description =
                "Accounting entry merchant does not match settlement merchant";

            finding.detectedAt =
                std::chrono::system_clock::now();

            findings.push_back(std::move(finding));
        }

        if (matchedEntry->amount !=
            settlement.netAmount)
        {
            const Money difference =
                matchedEntry->amount >=
                        settlement.netAmount
                    ? matchedEntry->amount -
                          settlement.netAmount
                    : settlement.netAmount -
                          matchedEntry->amount;

            ReconciliationFinding finding;

            finding.type = FindingType::AmountMismatch;
            finding.severity = FindingSeverity::High;

            finding.entityIds = {
                settlement.id,
                matchedEntry->id
            };

            finding.expectedValue =
                std::to_string(settlement.netAmount);

            finding.observedValue =
                std::to_string(matchedEntry->amount);

            finding.financialImpact =
                difference;

            finding.ruleId = id();

            finding.description =
                "Accounting entry amount does not match settlement net amount";

            finding.detectedAt =
                std::chrono::system_clock::now();

            findings.push_back(std::move(finding));
        }

        if (matchedEntry->type !=
            AccountingEntryType::Credit)
        {
            ReconciliationFinding finding;

            finding.type = FindingType::AmountMismatch;
            finding.severity = FindingSeverity::High;

            finding.entityIds = {
                settlement.id,
                matchedEntry->id
            };

            finding.expectedValue =
                "Credit accounting entry";

            finding.observedValue =
                "incorrect accounting entry type";

            finding.financialImpact =
                settlement.netAmount;

            finding.ruleId = id();

            finding.description =
                "Accounting entry type does not match expected settlement accounting type";

            finding.detectedAt =
                std::chrono::system_clock::now();

            findings.push_back(std::move(finding));
        }
    }

    return findings;
}

}