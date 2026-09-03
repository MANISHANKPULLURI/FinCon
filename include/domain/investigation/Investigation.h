#pragma once

#include "domain/investigation/InvestigationRecommendation.h"
#include "domain/investigation/InvestigationEvidence.h"
#include "domain/investigation/InvestigationHypothesis.h"
#include "domain/investigation/InvestigationToolCall.h"

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace fincon
{
    enum class InvestigationStatus
    {
        Pending,
        InProgress,
        EvidenceCollected,
        DecisionReady,
        Completed,
        Failed
    };

    enum class InvestigationOutcome
    {
        Unknown,
        AutoResolve,
        HumanReview,
        RequestMoreEvidence,
        Unresolved
    };

    enum class ConfidenceLevel
    {
        Unknown,
        Low,
        Medium,
        High
    };

    class Investigation
    {
    public:
        Investigation() = default;

        explicit Investigation(std::string id)
            : id_(std::move(id))
        {
        }

        const std::string& id() const
        {
            return id_;
        }

        const std::string& incidentId() const
        {
            return incidentId_;
        }

        InvestigationStatus status() const
        {
            return status_;
        }

        InvestigationOutcome outcome() const
        {
            return outcome_;
        }

        ConfidenceLevel confidence() const
        {
            return confidence_;
        }

        std::int64_t confirmedImpact() const
        {
            return confirmedImpact_;
        }

        const std::vector<std::string>& evidenceIds() const
        {
            return evidenceIds_;
        }

        const std::vector<std::string>& toolCallIds() const
        {
            return toolCallIds_;
        }

        const std::vector<std::string>& hypothesisIds() const
        {
            return hypothesisIds_;
        }

        bool llmEscalated() const
        {
            return llmEscalated_;
        }

        const InvestigationRecommendation* recommendation() const
        {
            return recommendation_ ? &(*recommendation_) : nullptr;
        }

        const std::vector<InvestigationEvidence>& evidence() const
        {
            return evidence_;
        }

        const std::vector<InvestigationHypothesis>& hypotheses() const
        {
            return hypotheses_;
        }

        const std::vector<InvestigationToolCall>& toolCalls() const
        {
            return toolCalls_;
        }

        void setIncidentId(std::string incidentId)
        {
            incidentId_ = std::move(incidentId);
        }

        void setStatus(InvestigationStatus status)
        {
            status_ = status;
        }

        void setOutcome(InvestigationOutcome outcome)
        {
            outcome_ = outcome;
        }

        void setConfidence(ConfidenceLevel confidence)
        {
            confidence_ = confidence;
        }

        void setConfirmedImpact(std::int64_t confirmedImpact)
        {
            confirmedImpact_ = confirmedImpact;
        }

        void addEvidence(std::string evidenceId)
        {
            evidenceIds_.push_back(std::move(evidenceId));
        }

        void addToolCall(std::string toolCallId)
        {
            toolCallIds_.push_back(std::move(toolCallId));
        }

        void addHypothesis(std::string hypothesisId)
        {
            hypothesisIds_.push_back(std::move(hypothesisId));
        }

        void setLlmEscalated(bool llmEscalated)
        {
            llmEscalated_ = llmEscalated;
        }

        void setRecommendation(
            InvestigationRecommendation recommendation
        )
        {
            recommendation_ = std::move(recommendation);
        }

        void setEvidence(
            std::vector<InvestigationEvidence> evidence)
        {
            evidence_ = std::move(evidence);
        }

        void setHypotheses(
            std::vector<InvestigationHypothesis> hypotheses)
        {
            hypotheses_ = std::move(hypotheses);
        }

        void setToolCalls(
            std::vector<InvestigationToolCall> toolCalls)
        {
            toolCalls_ = std::move(toolCalls);
        }

    private:
        std::string id_;
        std::string incidentId_;

        InvestigationStatus status_ = InvestigationStatus::Pending;
        InvestigationOutcome outcome_ = InvestigationOutcome::Unknown;
        ConfidenceLevel confidence_ = ConfidenceLevel::Unknown;

        std::int64_t confirmedImpact_ = 0;

        std::vector<std::string> evidenceIds_;
        std::vector<std::string> toolCallIds_;
        std::vector<std::string> hypothesisIds_;

        bool llmEscalated_ = false;

        std::optional<InvestigationRecommendation> recommendation_;
        std::vector<InvestigationEvidence> evidence_;
        std::vector<InvestigationHypothesis> hypotheses_;
        std::vector<InvestigationToolCall> toolCalls_;
    };
}