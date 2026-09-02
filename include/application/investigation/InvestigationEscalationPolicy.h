#pragma once

#include "domain/incident/Incident.h"
#include "domain/investigation/Investigation.h"

namespace fincon
{
    enum class EscalationDecision
    {
        ContinueDeterministic,
        EscalateToLLM
    };

    class InvestigationEscalationPolicy
    {
    public:
        virtual ~InvestigationEscalationPolicy() = default;

        virtual EscalationDecision evaluate(
            const Incident& incident,
            const Investigation& investigation) const = 0;
    };
}