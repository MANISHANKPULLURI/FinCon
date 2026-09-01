#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace fincon
{

    enum class IncidentStatus
    {
        InvestigationRequired,
        Investigating,
        Resolved,
        HumanReview,
        RequestMoreEvidence,
        Unresolved
    };

    enum class IncidentType
    {
        Unknown,
        SettlementIssue,
        RefundIssue,
        FeeIssue,
        BankIssue,
        AccountingIssue,
        MissingRecord,
        DuplicateRecord,
        TimingIssue
    };

    class Incident
    {
    public:

        Incident() = default;

        explicit Incident(
            std::string id)
            : id_(std::move(id))
        {
        }

        const std::string& id() const
        {
            return id_;
        }

        IncidentStatus status() const
        {
            return status_;
        }

        IncidentType type() const
        {
            return type_;
        }

        std::int64_t financialImpact() const
        {
            return financialImpact_;
        }

        const std::vector<std::string>& findingIds() const
        {
            return findingIds_;
        }

        const std::vector<std::string>& entityIds() const
        {
            return entityIds_;
        }

        void setStatus(
            IncidentStatus status)
        {
            status_ = status;
        }

        void setType(
            IncidentType type)
        {
            type_ = type;
        }

        void setFinancialImpact(
            std::int64_t financialImpact)
        {
            financialImpact_ = financialImpact;
        }

        void addFinding(
            const std::string& findingId)
        {
            findingIds_.push_back(findingId);
        }

        void addEntity(
            const std::string& entityId)
        {
            entityIds_.push_back(entityId);
        }

    private:

        std::string id_;

        IncidentStatus status_ =
            IncidentStatus::InvestigationRequired;

        IncidentType type_ =
            IncidentType::Unknown;

        std::int64_t financialImpact_ = 0;

        std::vector<std::string> findingIds_;

        std::vector<std::string> entityIds_;
    };

}