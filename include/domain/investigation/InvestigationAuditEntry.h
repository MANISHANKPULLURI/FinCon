#pragma once

#include <cstdint>
#include <string>
#include <utility>

namespace fincon
{
    enum class AuditEventType
    {
        InvestigationStarted,
        EvidenceCollected,
        ToolExecuted,
        HypothesisGenerated,
        ImpactCalculated,
        DecisionMade,
        RecommendationCreated,
        ActionRequested,
        ActionExecuted,
        InvestigationCompleted,
        InvestigationFailed
    };

    class InvestigationAuditEntry
    {
    public:
        InvestigationAuditEntry() = default;

        explicit InvestigationAuditEntry(std::string id)
            : id_(std::move(id))
        {
        }

        const std::string& id() const
        {
            return id_;
        }

        const std::string& investigationId() const
        {
            return investigationId_;
        }

        const std::string& incidentId() const
        {
            return incidentId_;
        }

        AuditEventType eventType() const
        {
            return eventType_;
        }

        const std::string& actor() const
        {
            return actor_;
        }

        const std::string& description() const
        {
            return description_;
        }

        std::int64_t financialImpact() const
        {
            return financialImpact_;
        }

        const std::string& timestamp() const
        {
            return timestamp_;
        }

        void setInvestigationId(std::string investigationId)
        {
            investigationId_ = std::move(investigationId);
        }

        void setIncidentId(std::string incidentId)
        {
            incidentId_ = std::move(incidentId);
        }

        void setEventType(AuditEventType eventType)
        {
            eventType_ = eventType;
        }

        void setActor(std::string actor)
        {
            actor_ = std::move(actor);
        }

        void setDescription(std::string description)
        {
            description_ = std::move(description);
        }

        void setFinancialImpact(std::int64_t financialImpact)
        {
            financialImpact_ = financialImpact;
        }

        void setTimestamp(std::string timestamp)
        {
            timestamp_ = std::move(timestamp);
        }

    private:
        std::string id_;
        std::string investigationId_;
        std::string incidentId_;
        AuditEventType eventType_ = AuditEventType::InvestigationStarted;
        std::string actor_;
        std::string description_;
        std::int64_t financialImpact_ = 0;
        std::string timestamp_;
    };
}