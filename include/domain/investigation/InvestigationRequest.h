#pragma once

#include "domain/incident/Incident.h"
#include "domain/investigation/InvestigationEvidence.h"
#include "domain/investigation/InvestigationHypothesis.h"
#include "domain/investigation/InvestigationToolCall.h"

#include <string>
#include <vector>
#include <utility>

namespace fincon
{
    class InvestigationRequest
    {
    public:
        InvestigationRequest() = default;

        explicit InvestigationRequest(std::string id)
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

        const Incident* incident() const
        {
            return incident_;
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

        void setIncident(const Incident* incident)
        {
            incident_ = incident;
        }

        void addEvidence(const InvestigationEvidence& evidence)
        {
            evidence_.push_back(evidence);
        }

        void addHypothesis(const InvestigationHypothesis& hypothesis)
        {
            hypotheses_.push_back(hypothesis);
        }

        void addToolCall(const InvestigationToolCall& toolCall)
        {
            toolCalls_.push_back(toolCall);
        }

    private:
        std::string id_;
        std::string incidentId_;
        const Incident* incident_ = nullptr;
        std::vector<InvestigationEvidence> evidence_;
        std::vector<InvestigationHypothesis> hypotheses_;
        std::vector<InvestigationToolCall> toolCalls_;
    };
}