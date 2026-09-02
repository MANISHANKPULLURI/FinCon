#include "application/evaluation/Phase2EvaluationRunner.h"

#include "application/evaluation/Phase2EvaluationScenario.h"

#include <utility>

namespace fincon
{
    Phase2EvaluationRunner::Phase2EvaluationRunner(
        const InvestigationService& investigationService,
        const Phase2Evaluation& evaluation)
        : investigationService_(investigationService),
          evaluation_(evaluation)
    {
    }

    Phase2EvaluationResult Phase2EvaluationRunner::run(
        const FinancialDataset& dataset,
        const std::vector<InjectedException>& exceptions,
        const std::vector<Incident>& incidents) const
    {
        std::vector<Investigation> investigations;
        investigations.reserve(incidents.size());

        for (const Incident& incident : incidents)
        {
            investigations.push_back(
                investigationService_.investigate(incident)
            );
        }

        Phase2EvaluationScenarioSet scenarioSet;

        const std::vector<Phase2EvaluationScenario> scenarios =
            scenarioSet.generate(exceptions.size());

        return evaluation_.evaluate(
            dataset.payments.size(),
            exceptions.size(),
            incidents,
            investigations,
            scenarios
        );
    }
}