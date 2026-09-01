#pragma once

#include "domain/incident/Incident.h"
#include "domain/investigation/Investigation.h"

#include <vector>

namespace fincon
{

    class InvestigationBuilder final
    {
    public:

        std::vector<Investigation> build(
            const std::vector<Incident>& incidents
        ) const;
    };

}