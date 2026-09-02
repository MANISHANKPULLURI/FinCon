#pragma once

#include "domain/investigation/InvestigationDecision.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace fincon
{
    enum class RecommendationAction
    {
        None,
        MarkResolved,
        RequestEvidence,
        EscalateToFinance,
        ApplyFinancialAdjustment
    };

    class InvestigationRecommendation
    {
    public:
        InvestigationRecommendation() = default;

        explicit InvestigationRecommendation(std::string id)
            : id_(std::move(id))
        {
        }

        const std::string& id() const
        {
            return id_;
        }

        RecommendationAction action() const
        {
            return action_;
        }

        DecisionConfidence confidence() const
        {
            return confidence_;
        }

        const std::string& rationale() const
        {
            return rationale_;
        }

        std::int64_t financialImpact() const
        {
            return financialImpact_;
        }

        const std::vector<std::string>& evidenceIds() const
        {
            return evidenceIds_;
        }

        void setAction(RecommendationAction action)
        {
            action_ = action;
        }

        void setConfidence(DecisionConfidence confidence)
        {
            confidence_ = confidence;
        }

        void setRationale(std::string rationale)
        {
            rationale_ = std::move(rationale);
        }

        void setFinancialImpact(std::int64_t financialImpact)
        {
            financialImpact_ = financialImpact;
        }

        void addEvidence(std::string evidenceId)
        {
            evidenceIds_.push_back(std::move(evidenceId));
        }

    private:
        std::string id_;
        RecommendationAction action_ = RecommendationAction::None;
        DecisionConfidence confidence_ = DecisionConfidence::Low;
        std::string rationale_;
        std::int64_t financialImpact_ = 0;
        std::vector<std::string> evidenceIds_;
    };
}