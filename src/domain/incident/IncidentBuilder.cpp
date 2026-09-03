#include "domain/incident/IncidentBuilder.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace fincon
{

    namespace
    {

        IncidentType determineIncidentType(
            const FindingCorrelation& correlation)
        {
            for (const std::string& ruleId : correlation.ruleIds)
            {
                if (ruleId == "DUPLICATE_RECORD_RECONCILIATION")
                    return IncidentType::DuplicateRecord;
                if (ruleId == "MISSING_SETTLEMENT_RECONCILIATION" || ruleId == "PAYMENT_SETTLEMENT_MATCHING")
                {
                    bool hasDup = false;
                    for (auto &r : correlation.ruleIds) if (r == "DUPLICATE_RECORD_RECONCILIATION") hasDup = true;
                    if (!hasDup) return IncidentType::MissingRecord;
                }
                if (ruleId == "SETTLEMENT_REFUND_RECONCILIATION" || ruleId == "REFUND_RECONCILIATION")
                    return IncidentType::RefundIssue;
                if (ruleId == "SETTLEMENT_FEE_RECONCILIATION")
                    return IncidentType::FeeIssue;
                if (ruleId == "SETTLEMENT_TIMING_RECONCILIATION")
                    return IncidentType::TimingIssue;
                if (ruleId == "SETTLEMENT_CALCULATION_RECONCILIATION")
                    return IncidentType::SettlementIssue;
                if (ruleId == "SETTLEMENT_BANK_RECONCILIATION")
                    return IncidentType::BankIssue;
                if (ruleId == "SETTLEMENT_ACCOUNTING_RECONCILIATION")
                    return IncidentType::AccountingIssue;
            }
            for (const std::string& fid : correlation.findingIds)
            {
                if (fid.rfind("FC-DUPLICATE",0)==0) return IncidentType::DuplicateRecord;
                if (fid.rfind("FC-MISSING",0)==0) return IncidentType::MissingRecord;
            }
            return IncidentType::Unknown;
        }

    }

    std::vector<Incident>
    IncidentBuilder::build(
        const std::vector<FindingCorrelation>& correlations) const
    {
        std::vector<Incident> incidents;

        std::size_t nextIncidentId = 1;

        for (const FindingCorrelation& correlation :
             correlations)
        {
            Incident incident(
                "INC-" +
                std::to_string(nextIncidentId++)
            );

            incident.setType(
                determineIncidentType(correlation)
            );

            incident.setStatus(
                IncidentStatus::InvestigationRequired
            );

            incident.setFinancialImpact(
                correlation.financialExposure
            );

            for (const std::string& findingId :
                 correlation.findingIds)
            {
                incident.addFinding(
                    findingId
                );
            }

            for (const std::string& entityId :
                 correlation.entityIds)
            {
                incident.addEntity(
                    entityId
                );
            }

            incidents.push_back(
                std::move(incident)
            );
        }

        return incidents;
    }

}