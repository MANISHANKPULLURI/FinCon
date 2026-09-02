#include "application/evaluation/Phase2Evaluation.h"
#include <algorithm>
namespace fincon
{
    Phase2EvaluationResult Phase2Evaluation::evaluate(
        std::size_t records,
        std::size_t injectedExceptions,
        const std::vector<Incident>& incidents,
        const std::vector<Investigation>& investigations,
        const std::vector<Phase2EvaluationScenario>& scenarios) const
    {
        Phase2EvaluationResult result;

        result.records = records;
        result.injectedExceptions = injectedExceptions;
        result.incidents = incidents.size();
        result.investigations = investigations.size();

        const std::size_t evaluationCount =
            std::min(
                scenarios.size(),
                std::min(
                    incidents.size(),
                    investigations.size()
                )
            );

        for (const Investigation& investigation : investigations)
        {
            switch (investigation.outcome())
            {
            case InvestigationOutcome::AutoResolve:
                ++result.autoResolved;
                break;

            case InvestigationOutcome::HumanReview:
                ++result.humanReview;
                break;

            case InvestigationOutcome::RequestMoreEvidence:
                ++result.requestMoreEvidence;
                break;

            case InvestigationOutcome::Unresolved:
                ++result.unresolved;
                break;

            case InvestigationOutcome::Unknown:
                break;
            }

            if (investigation.llmEscalated())
                ++result.llmResolutions;
            else
                ++result.deterministicResolutions;
        }

        for (std::size_t index = 0; index < evaluationCount; ++index)
        {
            const Incident& incident = incidents[index];
            const Investigation& investigation = investigations[index];
            const Phase2EvaluationScenario& scenario = scenarios[index];

            const bool correctIncidentType =
                incident.type() == scenario.expectedIncidentType;

            const bool correctOutcome =
                investigation.outcome() == scenario.expectedOutcome;

            const bool correctImpact =
                investigation.confirmedImpact() == scenario.expectedImpact;

            const bool correctLLMEscalation =
                investigation.llmEscalated() == scenario.requiresLLM;

            if (correctIncidentType)
                ++result.correctIncidentTypes;

            if (correctOutcome)
                ++result.correctOutcomes;

            if (correctImpact)
                ++result.correctImpacts;

            if (correctLLMEscalation)
                ++result.correctLLMEscalations;

            if (correctIncidentType &&
                correctOutcome &&
                correctImpact &&
                correctLLMEscalation)
            {
                ++result.correctlyEvaluated;
            }
        }

        result.passed =
            result.records >= 1000 &&
            result.injectedExceptions > 0 &&
            result.incidents == result.injectedExceptions &&
            result.investigations == result.incidents &&
            result.unresolved == 0 &&
            result.correctlyEvaluated == evaluationCount;

        return result;
    }
}