#pragma once

#include "application/evaluation/Phase2EvaluationScenario.h"
#include "domain/incident/Incident.h"
#include "domain/investigation/Investigation.h"

#include <cstddef>
#include <vector>

namespace fincon
{
    struct Phase2EvaluationResult
    {
        std::size_t records = 0;
        std::size_t injectedExceptions = 0;
        std::size_t incidents = 0;
        std::size_t investigations = 0;

        std::size_t deterministicResolutions = 0;
        std::size_t llmResolutions = 0;

        std::size_t autoResolved = 0;
        std::size_t humanReview = 0;
        std::size_t requestMoreEvidence = 0;
        std::size_t unresolved = 0;

        std::size_t correctIncidentTypes = 0;
        std::size_t correctOutcomes = 0;
        std::size_t correctImpacts = 0;
        std::size_t correctLLMEscalations = 0;
        std::size_t correctlyEvaluated = 0;

        bool passed = false;
    };

    class Phase2Evaluation
    {
    public:
        Phase2EvaluationResult evaluate(
            std::size_t records,
            std::size_t injectedExceptions,
            const std::vector<Incident>& incidents,
            const std::vector<Investigation>& investigations,
            const std::vector<Phase2EvaluationScenario>& scenarios
        ) const;
    };
}