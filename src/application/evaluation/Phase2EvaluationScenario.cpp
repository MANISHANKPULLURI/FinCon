#include "application/evaluation/Phase2EvaluationScenario.h"

namespace fincon
{
    std::vector<Phase2EvaluationScenario>
    Phase2EvaluationScenarioSet::generate(std::size_t count) const
    {
        const std::vector<Phase2EvaluationScenario> templates =
        {
            {
                "DUPLICATE",
                IncidentType::DuplicateRecord,
                InvestigationOutcome::HumanReview,
                3484321,
                true
            },
            {
                "MISSING",
                IncidentType::MissingRecord,
                InvestigationOutcome::RequestMoreEvidence,
                487433,
                true
            },
            {
                "TIMING_SETTLEMENT",
                IncidentType::TimingIssue,
                InvestigationOutcome::AutoResolve,
                0,
                false
            },
            {
                "REFUND_MISMATCH",
                IncidentType::RefundIssue,
                InvestigationOutcome::RequestMoreEvidence,
                1000,
                true
            },
            {
                "SETTLEMENT_PARTIAL",
                IncidentType::SettlementIssue,
                InvestigationOutcome::HumanReview,
                1867021,
                true
            },
            {
                "TIMING_BANK",
                IncidentType::TimingIssue,
                InvestigationOutcome::AutoResolve,
                0,
                false
            },
            {
                "FEE_DISCREPANCY",
                IncidentType::FeeIssue,
                InvestigationOutcome::RequestMoreEvidence,
                500,
                true
            },
            {
                "SETTLEMENT_UNRESOLVABLE",
                IncidentType::SettlementIssue,
                InvestigationOutcome::AutoResolve,
                1,
                true
            }
        };

        std::vector<Phase2EvaluationScenario> scenarios;
        scenarios.reserve(count);

        for (std::size_t index = 0; index < count; ++index)
        {
            Phase2EvaluationScenario scenario =
                templates[index % templates.size()];

            scenario.id =
                scenario.id + "-" + std::to_string(index + 1);

            scenarios.push_back(std::move(scenario));
        }

        return scenarios;
    }
}