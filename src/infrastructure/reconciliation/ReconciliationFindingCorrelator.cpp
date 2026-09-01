#include "infrastructure/reconciliation/ReconciliationFindingCorrelator.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fincon {

namespace {

bool containsEntity(
    const ReconciliationFinding& finding,
    const std::string& entityId)
{
    return std::find(
        finding.entityIds.begin(),
        finding.entityIds.end(),
        entityId
    ) != finding.entityIds.end();
}

bool sharesEntity(
    const ReconciliationFinding& first,
    const ReconciliationFinding& second)
{
    for (const std::string& entityId : first.entityIds)
    {
        if (containsEntity(second, entityId))
        {
            return true;
        }
    }

    return false;
}

bool isMissingSettlementFinding(
    const ReconciliationFinding& finding)
{
    return finding.ruleId ==
               "MISSING_SETTLEMENT_RECONCILIATION" ||
           (
               finding.ruleId ==
                   "PAYMENT_SETTLEMENT_MATCHING" &&
               finding.observedValue ==
                   "settlement missing"
           );
}

bool isDuplicateSettlementFinding(
    const ReconciliationFinding& finding)
{
    return finding.ruleId ==
               "DUPLICATE_RECORD_RECONCILIATION" ||
           (
               finding.ruleId ==
                   "PAYMENT_SETTLEMENT_MATCHING" &&
               finding.observedValue ==
                   "multiple settlements for payment"
           );
}

void mergeEntities(
    ReconciliationFinding& target,
    const ReconciliationFinding& source)
{
    for (const std::string& entityId : source.entityIds)
    {
        if (!containsEntity(target, entityId))
        {
            target.entityIds.push_back(entityId);
        }
    }

    std::sort(
        target.entityIds.begin(),
        target.entityIds.end()
    );
}

void mergeFinding(
    ReconciliationFinding& target,
    const ReconciliationFinding& source)
{
    mergeEntities(target, source);

    if (source.financialImpact >
        target.financialImpact)
    {
        target.financialImpact =
            source.financialImpact;
    }

    if (target.expectedValue.empty())
    {
        target.expectedValue =
            source.expectedValue;
    }

    if (target.observedValue.empty())
    {
        target.observedValue =
            source.observedValue;
    }
}

}

std::vector<ReconciliationFinding>
ReconciliationFindingCorrelator::correlate(
    const std::vector<ReconciliationFinding>& findings) const
{
    std::vector<ReconciliationFinding> correlated;

    for (const ReconciliationFinding& finding : findings)
    {
        bool merged = false;

        for (ReconciliationFinding& existing : correlated)
        {
            if (isMissingSettlementFinding(existing) &&
                isMissingSettlementFinding(finding))
            {
                if (sharesEntity(existing, finding))
                {
                    mergeFinding(existing, finding);
                    merged = true;
                    break;
                }
            }

            if (isDuplicateSettlementFinding(existing) &&
                isDuplicateSettlementFinding(finding))
            {
                if (sharesEntity(existing, finding))
                {
                    mergeFinding(existing, finding);
                    merged = true;
                    break;
                }
            }

            if (existing.ruleId == finding.ruleId &&
                existing.type == finding.type &&
                sharesEntity(existing, finding))
            {
                mergeFinding(existing, finding);
                merged = true;
                break;
            }
        }

        if (!merged)
        {
            ReconciliationFinding copy = finding;

            std::sort(
                copy.entityIds.begin(),
                copy.entityIds.end()
            );

            correlated.push_back(
                std::move(copy)
            );
        }
    }

    return correlated;
}

}