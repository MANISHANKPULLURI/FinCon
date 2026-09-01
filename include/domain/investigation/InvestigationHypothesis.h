#pragma once

#include "common/Money.h"

#include <string>
#include <utility>
#include <vector>

namespace fincon
{

    enum class HypothesisStatus
    {
        Proposed,
        Testing,
        Supported,
        Rejected,
        Inconclusive
    };

    class InvestigationHypothesis final
    {
    public:

        InvestigationHypothesis() = default;

        explicit InvestigationHypothesis(
            std::string id)
            : id_(std::move(id))
        {
        }

        const std::string& id() const
        {
            return id_;
        }

        const std::string& description() const
        {
            return description_;
        }

        HypothesisStatus status() const
        {
            return status_;
        }

        int confidenceScore() const
        {
            return confidenceScore_;
        }

        Money estimatedImpact() const
        {
            return estimatedImpact_;
        }

        const std::vector<std::string>& evidenceIds() const
        {
            return evidenceIds_;
        }

        void setDescription(
            std::string description)
        {
            description_ =
                std::move(description);
        }

        void setStatus(
            HypothesisStatus status)
        {
            status_ = status;
        }

        void setConfidenceScore(
            int confidenceScore)
        {
            confidenceScore_ =
                confidenceScore;
        }

        void setEstimatedImpact(
            Money estimatedImpact)
        {
            estimatedImpact_ =
                estimatedImpact;
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

        std::string description_;

        HypothesisStatus status_ =
            HypothesisStatus::Proposed;

        int confidenceScore_ = 0;

        Money estimatedImpact_{0};

        std::vector<std::string> evidenceIds_;
    };

}