#pragma once

#include "common/Money.h"

#include <string>
#include <utility>
#include <vector>

namespace fincon
{

    enum class DecisionType
    {
        AutoResolve,
        HumanReview,
        RequestMoreEvidence,
        Unresolved
    };

    enum class DecisionConfidence
    {
        Low,
        Medium,
        High
    };

    class InvestigationDecision final
    {
    public:

        InvestigationDecision() = default;

        explicit InvestigationDecision(
            std::string id)
            : id_(std::move(id))
        {
        }

        const std::string& id() const
        {
            return id_;
        }

        DecisionType type() const
        {
            return type_;
        }

        DecisionConfidence confidence() const
        {
            return confidence_;
        }

        const std::string& rationale() const
        {
            return rationale_;
        }

        Money financialImpact() const
        {
            return financialImpact_;
        }

        const std::vector<std::string>& evidenceIds() const
        {
            return evidenceIds_;
        }

        void setType(
            DecisionType type)
        {
            type_ = type;
        }

        void setConfidence(
            DecisionConfidence confidence)
        {
            confidence_ = confidence;
        }

        void setRationale(
            std::string rationale)
        {
            rationale_ =
                std::move(rationale);
        }

        void setFinancialImpact(
            Money financialImpact)
        {
            financialImpact_ =
                financialImpact;
        }

        void addEvidence(
            std::string evidenceId)
        {
            evidenceIds_.push_back(
                std::move(evidenceId)
            );
        }

    private:

        std::string id_;

        DecisionType type_ =
            DecisionType::Unresolved;

        DecisionConfidence confidence_ =
            DecisionConfidence::Low;

        std::string rationale_;

        Money financialImpact_{0};

        std::vector<std::string> evidenceIds_;
    };

}