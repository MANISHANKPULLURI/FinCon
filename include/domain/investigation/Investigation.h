#pragma once

#include "common/Money.h"

#include <cstdint>
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

    class Investigation final
    {
    public:

        Investigation() = default;

        explicit Investigation(
            std::string id)
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

        Money confirmedImpact() const
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

        void setIncidentId(
            std::string incidentId)
        {
            incidentId_ =
                std::move(incidentId);
        }

        void setStatus(
            InvestigationStatus status)
        {
            status_ = status;
        }

        void setOutcome(
            InvestigationOutcome outcome)
        {
            outcome_ = outcome;
        }

        void setConfidence(
            ConfidenceLevel confidence)
        {
            confidence_ = confidence;
        }

        void setConfirmedImpact(
            Money confirmedImpact)
        {
            confirmedImpact_ =
                confirmedImpact;
        }

        void addEvidence(
            std::string evidenceId)
        {
            evidenceIds_.push_back(
                std::move(evidenceId)
            );
        }

        void addToolCall(
            std::string toolCallId)
        {
            toolCallIds_.push_back(
                std::move(toolCallId)
            );
        }

        void addHypothesis(
            std::string hypothesisId)
        {
            hypothesisIds_.push_back(
                std::move(hypothesisId)
            );
        }

    private:

        std::string id_;

        std::string incidentId_;

        InvestigationStatus status_ =
            InvestigationStatus::Pending;

        InvestigationOutcome outcome_ =
            InvestigationOutcome::Unknown;

        ConfidenceLevel confidence_ =
            ConfidenceLevel::Unknown;

        Money confirmedImpact_{0};

        std::vector<std::string> evidenceIds_;

        std::vector<std::string> toolCallIds_;

        std::vector<std::string> hypothesisIds_;
    };

}