#include "infrastructure/reconciliation/rules/DuplicateRecordRule.h"

#include <chrono>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fincon {

std::string DuplicateRecordRule::id() const
{
    return "DUPLICATE_RECORD_RECONCILIATION";
}

std::vector<ReconciliationFinding>
DuplicateRecordRule::evaluate(
    const FinancialDataset& dataset) const
{
    std::vector<ReconciliationFinding> findings;

    std::unordered_map<std::string, std::vector<const Settlement*>>
        settlementGroups;

    for (const Settlement& settlement :
         dataset.settlements)
    {
        if (settlement.paymentIds.empty())
        {
            continue;
        }

        std::string key =
            settlement.merchantId + "|";

        for (const std::string& paymentId :
             settlement.paymentIds)
        {
            key += paymentId;
            key += "|";
        }

        settlementGroups[key].push_back(
            &settlement
        );
    }

    for (const auto& [key, settlements] :
         settlementGroups)
    {
        if (settlements.size() < 2)
        {
            continue;
        }

        for (std::size_t index = 1;
             index < settlements.size();
             ++index)
        {
            const Settlement& original =
                *settlements[0];

            const Settlement& duplicate =
                *settlements[index];

            ReconciliationFinding finding;

            finding.type =
                FindingType::DuplicateRecord;

            finding.severity =
                FindingSeverity::High;

            finding.entityIds = {
                original.id,
                duplicate.id
            };

            finding.expectedValue =
                "one settlement for payment";

            finding.observedValue =
                "multiple settlements for same payment";

            finding.financialImpact =
                duplicate.netAmount;

            finding.ruleId = id();

            finding.description =
                "Multiple settlement records reference "
                "the same payment";

            finding.detectedAt =
                std::chrono::system_clock::now();

            findings.push_back(
                std::move(finding)
            );
        }
    }

    return findings;
}

}