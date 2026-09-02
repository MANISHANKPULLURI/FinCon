#include "application/investigation/DefaultInvestigationService.h"

#include <string>
#include <utility>
#include <vector>

namespace fincon
{
    Investigation DefaultInvestigationService::investigate(
        const Incident& incident)
    {
        Investigation investigation(
            "INV-" + incident.id()
        );

        investigation.setIncidentId(
            incident.id()
        );

        investigation.setStatus(
            InvestigationStatus::InProgress
        );

        InvestigationContext context;
        context.incident = &incident;

        const std::vector<InvestigationToolCall> plannedCalls =
            planner_.plan(context);

        for (const InvestigationToolCall& plannedCall :
             plannedCalls)
        {
            const InvestigationTool* tool =
                toolRegistry_.get(
                    plannedCall.toolName()
                );

            if (tool == nullptr)
            {
                InvestigationToolCall failedCall =
                    plannedCall;

                failedCall.setStatus(
                    ToolCallStatus::Failed
                );

                failedCall.setResult(
                    "Investigation tool not registered: " +
                    plannedCall.toolName()
                );

                context.toolCalls.push_back(
                    std::move(failedCall)
                );

                continue;
            }

            InvestigationToolCall executedCall =
                tool->execute(
                    plannedCall
                );

            context.toolCalls.push_back(
                std::move(executedCall)
            );
        }

        context.evidence =
            evidenceProvider_.collect(
                incident.entityIds()
            );

        context.hypotheses =
            hypothesisEngine_.generate(
                incident,
                context.evidence
            );

        const Money confirmedImpact =
            impactCalculator_.calculate(
                incident,
                context.evidence,
                context.hypotheses
            );

        InvestigationDecision decision =
            decisionPolicy_.decide(
                incident,
                context.evidence,
                context.hypotheses,
                confirmedImpact
            );

        for (const InvestigationEvidence& item :
             context.evidence)
        {
            investigation.addEvidence(
                item.id()
            );
        }

        for (const InvestigationHypothesis& hypothesis :
             context.hypotheses)
        {
            investigation.addHypothesis(
                hypothesis.id()
            );
        }

        for (const InvestigationToolCall& toolCall :
             context.toolCalls)
        {
            investigation.addToolCall(
                toolCall.id()
            );
        }

        investigation.setConfirmedImpact(
            decision.financialImpact()
        );

        investigation.setOutcome(
            [&decision]()
            {
                switch (decision.type())
                {
                case DecisionType::AutoResolve:
                    return InvestigationOutcome::AutoResolve;

                case DecisionType::HumanReview:
                    return InvestigationOutcome::HumanReview;

                case DecisionType::RequestMoreEvidence:
                    return InvestigationOutcome::RequestMoreEvidence;

                case DecisionType::Unresolved:
                    return InvestigationOutcome::Unresolved;
                }

                return InvestigationOutcome::Unknown;
            }()
        );

        investigation.setConfidence(
            [&decision]()
            {
                switch (decision.confidence())
                {
                case DecisionConfidence::Low:
                    return ConfidenceLevel::Low;

                case DecisionConfidence::Medium:
                    return ConfidenceLevel::Medium;

                case DecisionConfidence::High:
                    return ConfidenceLevel::High;
                }

                return ConfidenceLevel::Unknown;
            }()
        );

        investigation.setStatus(
            InvestigationStatus::Completed
        );

        return investigation;
    }

    DefaultInvestigationService::DefaultInvestigationService(
        const InvestigationPlanner& planner,
        const EvidenceProvider& evidenceProvider,
        const HypothesisEngine& hypothesisEngine,
        const ImpactCalculator& impactCalculator,
        const DecisionPolicy& decisionPolicy,
        const InvestigationToolRegistry& toolRegistry)
        : planner_(planner),
          evidenceProvider_(evidenceProvider),
          hypothesisEngine_(hypothesisEngine),
          impactCalculator_(impactCalculator),
          decisionPolicy_(decisionPolicy),
          toolRegistry_(toolRegistry)
    {
    }
}