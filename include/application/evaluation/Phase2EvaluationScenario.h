#pragma once

#include "domain/incident/Incident.h"
#include "domain/investigation/Investigation.h"

#include <cstddef>
#include <string>
#include <vector>

namespace fincon
{
    struct Phase2EvaluationScenario
    {
        std::string id;
        IncidentType expectedIncidentType = IncidentType::Unknown;
        InvestigationOutcome expectedOutcome = InvestigationOutcome::Unknown;
        Money expectedImpact = 0;
        bool requiresLLM = false;
    };

    class Phase2EvaluationScenarioSet
    {
    public:
        std::vector<Phase2EvaluationScenario> generate(
            std::size_t count
        ) const;
    };
}