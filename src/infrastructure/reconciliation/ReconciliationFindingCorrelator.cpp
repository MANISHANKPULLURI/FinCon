#include "infrastructure/reconciliation/ReconciliationFindingCorrelator.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace fincon
{

    namespace
    {

        bool contains(
            const std::string& value,
            const std::string& text)
        {
            return value.find(text) != std::string::npos;
        }

        bool isDuplicateFinding(
            const ReconciliationFinding& finding)
        {
            if (finding.ruleId ==
                "DUPLICATE_RECORD_RECONCILIATION")
            {
                return true;
            }

            if (contains(
                    finding.description,
                    "multiple settlement") ||
                contains(
                    finding.description,
                    "multiple settlements"))
            {
                return true;
            }

            for (const std::string& entityId :
                 finding.entityIds)
            {
                if (entityId.size() > 4 &&
                    entityId.rfind("-DUP") ==
                        entityId.size() - 4)
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
                       contains(
                           finding.description,
                           "does not have a corresponding settlement")
                   );
        }

        bool isSettlementFinding(
            const ReconciliationFinding& finding)
        {
            return std::any_of(
                finding.entityIds.begin(),
                finding.entityIds.end(),
                [](const std::string& entityId)
                {
                    return entityId.rfind("S-", 0) == 0;
                });
        }

        std::string settlementFromPayment(
            const std::string& paymentId)
        {
            if (paymentId.rfind("P-", 0) != 0)
            {
                return {};
            }

            return "S-" + paymentId.substr(2);
        }

        std::string baseSettlement(
            const std::string& settlementId)
        {
            if (settlementId.size() > 4 &&
                settlementId.rfind("-DUP") ==
                    settlementId.size() - 4)
            {
                return settlementId.substr(
                    0,
                    settlementId.size() - 4
                );
            }

            return settlementId;
        }

        std::string duplicateCorrelationKey(
            const ReconciliationFinding& finding)
        {
            std::string canonicalSettlement;

            for (const std::string& entityId :
                 finding.entityIds)
            {
                if (entityId.rfind("S-", 0) == 0)
                {
                    const std::string candidate =
                        baseSettlement(entityId);

                    if (!candidate.empty())
                    {
                        canonicalSettlement =
                            candidate;

                        if (entityId.size() > 4 &&
                            entityId.rfind("-DUP") ==
                                entityId.size() - 4)
                        {
                            break;
                        }
                    }
                }
            }

            if (!canonicalSettlement.empty())
            {
                return "DUPLICATE:" +
                       canonicalSettlement;
            }

            for (const std::string& entityId :
                 finding.entityIds)
            {
                if (entityId.rfind("P-", 0) == 0)
                {
                    const std::string settlementId =
                        settlementFromPayment(entityId);

                    if (!settlementId.empty())
                    {
                        return "DUPLICATE:" +
                               settlementId;
                    }
                }
            }

            return {};
        }

        std::string settlementCorrelationKey(
            const ReconciliationFinding& finding)
        {
            for (const std::string& entityId :
                 finding.entityIds)
            {
                if (entityId.rfind("S-", 0) == 0)
                {
                    return "SETTLEMENT:" +
                           baseSettlement(entityId);
                }
            }

            return {};
        }

        std::string missingSettlementCorrelationKey(
            const ReconciliationFinding& finding)
        {
            for (const std::string& entityId :
                 finding.entityIds)
            {
                if (entityId.rfind("P-", 0) == 0)
                {
                    return "MISSING:" + entityId;
                }
            }

            return {};
        }

        std::string fallbackCorrelationKey(
            const ReconciliationFinding& finding)
        {
            std::vector<std::string> entities =
                finding.entityIds;

            std::sort(
                entities.begin(),
                entities.end()
            );

            std::string key = "FINDING:";

            for (const std::string& entityId :
                 entities)
            {
                key += entityId;
                key += '|';
            }

            return key;
        }

        std::string correlationKey(
            const ReconciliationFinding& finding)
        {
            if (isDuplicateFinding(finding))
            {
                return duplicateCorrelationKey(finding);
            }

            if (isMissingSettlementFinding(finding))
            {
                return missingSettlementCorrelationKey(finding);
            }

            if (isSettlementFinding(finding))
            {
                return settlementCorrelationKey(finding);
            }

            return fallbackCorrelationKey(finding);
        }

        std::int64_t candidateExposure(
            const std::vector<const ReconciliationFinding*>& findings)
        {
            for (const ReconciliationFinding* finding :
                 findings)
            {
                if (finding->ruleId ==
                    "DUPLICATE_RECORD_RECONCILIATION")
                {
                    return finding->financialImpact;
                }
            }

            std::int64_t exposure = 0;

            for (const ReconciliationFinding* finding :
                 findings)
            {
                exposure = std::max(
                    exposure,
                    finding->financialImpact
                );
            }

            return exposure;
        }

    }

    std::vector<FindingCorrelation>
    ReconciliationFindingCorrelator::correlate(
        const std::vector<ReconciliationFinding>& findings) const
    {
        std::map<
            std::string,
            std::vector<const ReconciliationFinding*>
        > groups;

        for (const ReconciliationFinding& finding :
             findings)
        {
            std::string key =
                correlationKey(finding);

            if (key.empty())
            {
                key = fallbackCorrelationKey(finding);
            }

            groups[key].push_back(&finding);
        }

        std::vector<FindingCorrelation> correlations;

        std::size_t nextId = 1;

        for (const auto& [key, groupedFindings] :
             groups)
        {
            FindingCorrelation correlation;

            if (key.rfind("DUPLICATE:", 0) == 0)
            {
                correlation.id =
                    "FC-DUPLICATE-" +
                    std::to_string(nextId);
            }
            else if (key.rfind("MISSING:", 0) == 0)
            {
                correlation.id =
                    "FC-MISSING-" +
                    std::to_string(nextId);
            }
            else if (key.rfind("SETTLEMENT:", 0) == 0)
            {
                correlation.id =
                    "FC-SETTLEMENT-" +
                    std::to_string(nextId);
            }
            else
            {
                correlation.id =
                    "FC-" +
                    std::to_string(nextId);
            }

            ++nextId;

            std::vector<std::string> findingIds;
            std::vector<std::string> entityIds;
            std::vector<std::string> ruleIds;

            for (const ReconciliationFinding* finding :
                  groupedFindings)
            {
                findingIds.push_back(
                    finding->id
                );

                entityIds.insert(
                    entityIds.end(),
                    finding->entityIds.begin(),
                    finding->entityIds.end()
                );

                ruleIds.push_back(finding->ruleId);
            }

            std::sort(
                findingIds.begin(),
                findingIds.end()
            );

            findingIds.erase(
                std::unique(
                    findingIds.begin(),
                    findingIds.end()
                ),
                findingIds.end()
            );

            std::sort(
                entityIds.begin(),
                entityIds.end()
            );

            entityIds.erase(
                std::unique(
                    entityIds.begin(),
                    entityIds.end()
                ),
                entityIds.end()
            );

            correlation.findingIds =
                std::move(findingIds);

            correlation.entityIds =
                std::move(entityIds);

            std::sort(ruleIds.begin(), ruleIds.end());
            ruleIds.erase(std::unique(ruleIds.begin(), ruleIds.end()), ruleIds.end());
            correlation.ruleIds = std::move(ruleIds);

            correlation.financialExposure =
                Money(
                    candidateExposure(
                        groupedFindings
                    )
                );

            correlations.push_back(
                std::move(correlation)
            );
        }

        return correlations;
    }

}