#pragma once

#include "domain/incident/Incident.h"
#include "domain/investigation/InvestigationEvidence.h"
#include "domain/investigation/InvestigationHypothesis.h"
#include "domain/investigation/InvestigationToolCall.h"

#include <vector>

namespace fincon
{

    struct InvestigationContext
    {
        const Incident* incident = nullptr;

        std::vector<InvestigationEvidence> evidence;
        std::vector<InvestigationHypothesis> hypotheses;
        std::vector<InvestigationToolCall> toolCalls;
    };

}