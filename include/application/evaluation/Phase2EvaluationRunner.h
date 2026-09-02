#pragma once

#include "application/evaluation/Phase2Evaluation.h"
#include "application/evaluation/Phase2EvaluationScenario.h"
#include "application/investigation/InvestigationService.h"
#include "infrastructure/generator/FinancialDataGenerator.h"
#include "infrastructure/exception/ExceptionInjector.h"

#include <cstddef>

namespace fincon
{
    class Phase2EvaluationRunner
    {
    public:
        Phase2EvaluationRunner(
            const InvestigationService& investigationService,
            const Phase2Evaluation& evaluation
        );

        Phase2EvaluationResult run(
            const FinancialDataset& dataset,
            const std::vector<InjectedException>& exceptions,
            const std::vector<Incident>& incidents
        ) const;

    private:
        const InvestigationService& investigationService_;
        const Phase2Evaluation& evaluation_;
    };
}