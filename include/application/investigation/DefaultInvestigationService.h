#pragma once

#include "application/investigation/DecisionPolicy.h"
#include "application/investigation/EvidenceProvider.h"
#include "application/investigation/HypothesisEngine.h"
#include "application/investigation/ImpactCalculator.h"
#include "application/investigation/InvestigationCompletenessEvaluator.h"
#include "application/investigation/InvestigationEscalationPolicy.h"
#include "application/investigation/InvestigationPlanner.h"
#include "application/investigation/InvestigationToolRegistry.h"
#include "application/investigation/InvestigationAgentOrchestrator.h"
#include "application/investigation/InvestigationService.h"

namespace fincon
{
    class DefaultInvestigationService final : public InvestigationService
    {
    public:
        DefaultInvestigationService(
            const InvestigationPlanner& planner,
            const EvidenceProvider& evidenceProvider,
            const HypothesisEngine& hypothesisEngine,
            const ImpactCalculator& impactCalculator,
            const DecisionPolicy& decisionPolicy,
            const InvestigationToolRegistry& toolRegistry,
            const InvestigationCompletenessEvaluator& completenessEvaluator,
            const InvestigationEscalationPolicy& escalationPolicy,
            const InvestigationAgentOrchestrator& agentOrchestrator
        );

        Investigation investigate(
            const Incident& incident
        ) override;

    private:
        const InvestigationPlanner& planner_;
        const EvidenceProvider& evidenceProvider_;
        const HypothesisEngine& hypothesisEngine_;
        const ImpactCalculator& impactCalculator_;
        const DecisionPolicy& decisionPolicy_;
        const InvestigationToolRegistry& toolRegistry_;
        const InvestigationCompletenessEvaluator& completenessEvaluator_;
        const InvestigationEscalationPolicy& escalationPolicy_;
        const InvestigationAgentOrchestrator& agentOrchestrator_;
    };
}