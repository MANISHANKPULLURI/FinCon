#pragma once

#include "domain/investigation/InvestigationDecision.h"
#include "domain/investigation/InvestigationHypothesis.h"
#include "domain/investigation/InvestigationToolRequest.h"

#include <string>
#include <utility>
#include <vector>

namespace fincon
{
    class InvestigationResponse
    {
    public:
        InvestigationResponse() = default;

        explicit InvestigationResponse(std::string id)
            : id_(std::move(id))
        {
        }

        const std::string& id() const
        {
            return id_;
        }

        const InvestigationDecision& decision() const
        {
            return decision_;
        }

        const std::vector<InvestigationHypothesis>& hypotheses() const
        {
            return hypotheses_;
        }

        const std::vector<InvestigationToolRequest>& requestedToolCalls() const
        {
            return requestedToolCalls_;
        }

        const std::string& reasoning() const
        {
            return reasoning_;
        }

        const std::string& missingEvidence() const
        {
            return missingEvidence_;
        }

        bool requiresMoreEvidence() const
        {
            return requiresMoreEvidence_;
        }

        void setDecision(InvestigationDecision decision)
        {
            decision_ = std::move(decision);
        }

        void addHypothesis(InvestigationHypothesis hypothesis)
        {
            hypotheses_.push_back(
                std::move(hypothesis)
            );
        }

        void addRequestedToolCall(
            InvestigationToolRequest toolRequest)
        {
            requestedToolCalls_.push_back(
                std::move(toolRequest)
            );
        }

        void setReasoning(std::string reasoning)
        {
            reasoning_ = std::move(reasoning);
        }

        void setMissingEvidence(std::string missingEvidence)
        {
            missingEvidence_ = std::move(missingEvidence);
        }

        void setRequiresMoreEvidence(
            bool requiresMoreEvidence)
        {
            requiresMoreEvidence_ = requiresMoreEvidence;
        }

    private:
        std::string id_;
        InvestigationDecision decision_;
        std::vector<InvestigationHypothesis> hypotheses_;
        std::vector<InvestigationToolRequest> requestedToolCalls_;
        std::string reasoning_;
        std::string missingEvidence_;
        bool requiresMoreEvidence_ = false;
    };
}