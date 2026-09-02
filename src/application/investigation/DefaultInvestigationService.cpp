
#include "application/investigation/DefaultInvestigationService.h"

#include "application/investigation/InvestigationContext.h"
#include "application/investigation/InvestigationTool.h"

#include "domain/investigation/Investigation.h"
#include "domain/investigation/InvestigationDecision.h"
#include "domain/investigation/InvestigationEvidence.h"
#include "domain/investigation/InvestigationHypothesis.h"
#include "domain/investigation/InvestigationRequest.h"
#include "domain/investigation/InvestigationResponse.h"
#include "domain/investigation/InvestigationToolCall.h"

#include <string>
#include <utility>
#include <vector>

namespace fincon
{
    Investigation DefaultInvestigationService::investigate(
        const Incident& incident) const
    {
        Investigation investigation("INV-" + incident.id());
        investigation.setIncidentId(incident.id());
        investigation.setStatus(InvestigationStatus::InProgress);

        auditService_.record(
            investigation.id(),
            incident.id(),
            AuditEventType::InvestigationStarted,
            "investigation-service",
            "Investigation started for incident."
        );

        InvestigationContext context;
        context.incident = &incident;

        const std::vector<InvestigationToolCall> plannedCalls =
            planner_.plan(context);

        for (const InvestigationToolCall& plannedCall : plannedCalls)
        {
            const InvestigationTool* tool =
                toolRegistry_.get(plannedCall.toolName());

            if (tool == nullptr)
            {
                InvestigationToolCall failedCall = plannedCall;
                failedCall.setStatus(ToolCallStatus::Failed);
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
                tool->execute(plannedCall);

            auditService_.record(
                investigation.id(),
                incident.id(),
                AuditEventType::ToolExecuted,
                "investigation-service",
                executedCall.toolName() +
                    " tool execution completed."
            );

            context.toolCalls.push_back(
                std::move(executedCall)
            );
        }

        context.evidence =
            evidenceProvider_.collect(
                incident.entityIds()
            );

        for (const InvestigationEvidence& item : context.evidence)
        {
            auditService_.record(
                investigation.id(),
                incident.id(),
                AuditEventType::EvidenceCollected,
                "evidence-provider",
                "Evidence collected: " + item.id(),
                item.financialImpact()
            );
        }

        context.hypotheses =
            hypothesisEngine_.generate(
                incident,
                context.evidence
            );

        for (const InvestigationHypothesis& hypothesis :
             context.hypotheses)
        {
            auditService_.record(
                investigation.id(),
                incident.id(),
                AuditEventType::HypothesisGenerated,
                "hypothesis-engine",
                "Hypothesis generated: " + hypothesis.id(),
                hypothesis.estimatedImpact()
            );
        }

        const Money confirmedImpact =
            impactCalculator_.calculate(
                incident,
                context.evidence,
                context.hypotheses
            );

        auditService_.record(
            investigation.id(),
            incident.id(),
            AuditEventType::ImpactCalculated,
            "impact-calculator",
            "Financial impact calculated.",
            confirmedImpact
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

        auditService_.record(
            investigation.id(),
            incident.id(),
            AuditEventType::DecisionMade,
            "decision-policy",
            decision.rationale(),
            decision.financialImpact()
        );

        const InvestigationCompleteness completeness =
            completenessEvaluator_.evaluate(
                investigation
            );

        if (decision.type() == DecisionType::AutoResolve &&
            decision.confidence() == DecisionConfidence::High &&
            decision.financialImpact() == 0 &&
            !context.evidence.empty() &&
            !context.toolCalls.empty())
        {
            investigation.setStatus(
                InvestigationStatus::Completed
            );

            auditService_.record(
                investigation.id(),
                incident.id(),
                AuditEventType::InvestigationCompleted,
                "investigation-service",
                "Investigation completed deterministically.",
                investigation.confirmedImpact()
            );

            return investigation;
        }

        if (completeness == InvestigationCompleteness::Incomplete)
        {
            investigation.setOutcome(
                InvestigationOutcome::Unknown
            );
        }

        const EscalationDecision escalation =
            escalationPolicy_.evaluate(
                incident,
                investigation
            );

        if (escalation == EscalationDecision::EscalateToLLM)
        {
            investigation.setLlmEscalated(true);

            auditService_.record(
                investigation.id(),
                incident.id(),
                AuditEventType::InvestigationStarted,
                "llm-escalation",
                "Investigation escalated to LLM."
            );

            InvestigationRequest request(
                "REQ-" + incident.id()
            );

            request.setIncidentId(
                incident.id()
            );

            request.setIncident(
                &incident
            );

            for (const InvestigationEvidence& item :
                 context.evidence)
            {
                request.addEvidence(item);
            }

            for (const InvestigationHypothesis& hypothesis :
                 context.hypotheses)
            {
                request.addHypothesis(hypothesis);
            }

            for (const InvestigationToolCall& toolCall :
                 context.toolCalls)
            {
                request.addToolCall(toolCall);
            }

            const InvestigationResponse response =
                agentOrchestrator_.investigate(
                    std::move(request)
                );

            investigation.setConfirmedImpact(
                response.decision().financialImpact()
            );

            investigation.setOutcome(
                [&response]()
                {
                    switch (response.decision().type())
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
                [&response]()
                {
                    switch (response.decision().confidence())
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

            auditService_.record(
                investigation.id(),
                incident.id(),
                AuditEventType::DecisionMade,
                "llm-agent",
                response.decision().rationale(),
                response.decision().financialImpact()
            );
        }

        investigation.setStatus(
            InvestigationStatus::Completed
        );

        auditService_.record(
            investigation.id(),
            incident.id(),
            AuditEventType::InvestigationCompleted,
            "investigation-service",
            "Investigation completed.",
            investigation.confirmedImpact()
        );

        return investigation;
    }

    DefaultInvestigationService::DefaultInvestigationService(
        const InvestigationPlanner& planner,
        const EvidenceProvider& evidenceProvider,
        const HypothesisEngine& hypothesisEngine,
        const ImpactCalculator& impactCalculator,
        const DecisionPolicy& decisionPolicy,
        const InvestigationToolRegistry& toolRegistry,
        const InvestigationCompletenessEvaluator& completenessEvaluator,
        const InvestigationEscalationPolicy& escalationPolicy,
        const InvestigationAgentOrchestrator& agentOrchestrator,
        InvestigationAuditService& auditService,
        const RecommendationPolicy& recommendationPolicy)
        : planner_(planner),
          evidenceProvider_(evidenceProvider),
          hypothesisEngine_(hypothesisEngine),
          impactCalculator_(impactCalculator),
          decisionPolicy_(decisionPolicy),
          toolRegistry_(toolRegistry),
          completenessEvaluator_(completenessEvaluator),
          escalationPolicy_(escalationPolicy),
          agentOrchestrator_(agentOrchestrator),
          auditService_(auditService),
          recommendationPolicy_(recommendationPolicy)
    {
    }
}