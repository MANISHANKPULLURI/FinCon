#pragma once

#include "domain/incident/Incident.h"
#include "domain/investigation/Investigation.h"

namespace fincon
{

    class InvestigationService
    {
    public:
        virtual ~InvestigationService() = default;

        virtual Investigation investigate(
            const Incident& incident
        ) const = 0;
    };

}