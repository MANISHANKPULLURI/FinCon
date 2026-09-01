#include "domain/investigation/InvestigationBuilder.h"

#include <string>
#include <utility>
#include <vector>

namespace fincon
{

    std::vector<Investigation>
    InvestigationBuilder::build(
        const std::vector<Incident>& incidents) const
    {
        std::vector<Investigation> investigations;
        investigations.reserve(incidents.size());

        std::size_t nextInvestigationId = 1;

        for (const Incident& incident : incidents)
        {
            Investigation investigation(
                "INV-" +
                std::to_string(nextInvestigationId++)
            );

            investigation.setIncidentId(
                incident.id()
            );

            investigation.setStatus(
                InvestigationStatus::Pending
            );

            investigation.setOutcome(
                InvestigationOutcome::Unknown
            );

            investigation.setConfidence(
                ConfidenceLevel::Unknown
            );

            investigation.setConfirmedImpact(
                Money{0}
            );

            investigations.push_back(
                std::move(investigation)
            );
        }

        return investigations;
    }

}