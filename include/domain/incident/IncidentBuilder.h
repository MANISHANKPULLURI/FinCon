#pragma once

#include "domain/incident/FindingCorrelation.h"
#include "domain/incident/Incident.h"

#include <vector>

namespace fincon
{

    class IncidentBuilder final
    {
    public:

        std::vector<Incident> build(
            const std::vector<FindingCorrelation>& correlations
        ) const;
    };

}