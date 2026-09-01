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
            for (const std::string& findingId :
                 correlation.findingIds)
            {
                if (findingId == "RF-11" ||
                    findingId == "RF-13" ||
                    findingId == "RF-7" ||
                    findingId == "RF-16")
                {
                    return IncidentType::DuplicateRecord;
                }

                if (findingId == "RF-10" ||
                    findingId == "RF-12")
                {
                    return IncidentType::MissingRecord;
                }

                if (findingId == "RF-3")
                {
                    return IncidentType::RefundIssue;
                }

                if (findingId == "RF-4")
                {
                    return IncidentType::FeeIssue;
                }

                if (findingId == "RF-8" ||
                    findingId == "RF-9")
                {
                    return IncidentType::TimingIssue;
                }

                if (findingId == "RF-1" ||
                    findingId == "RF-2")
                {
                    return IncidentType::SettlementIssue;
                }
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