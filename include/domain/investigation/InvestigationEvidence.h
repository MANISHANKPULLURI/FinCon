#pragma once

#include "common/Money.h"

#include <string>
#include <utility>

namespace fincon
{

    enum class EvidenceType
    {
        Payment,
        Settlement,
        Refund,
        BankTransaction,
        AccountingEntry,
        RelatedTransaction,
        ReconciliationFinding,
        ExternalReference
    };

    enum class EvidenceStrength
    {
        Weak,
        Moderate,
        Strong,
        Conclusive
    };

    class InvestigationEvidence final
    {
    public:

        InvestigationEvidence() = default;

        explicit InvestigationEvidence(
            std::string id)
            : id_(std::move(id))
        {
        }

        const std::string& id() const
        {
            return id_;
        }

        EvidenceType type() const
        {
            return type_;
        }

        EvidenceStrength strength() const
        {
            return strength_;
        }

        const std::string& sourceId() const
        {
            return sourceId_;
        }

        const std::string& description() const
        {
            return description_;
        }

        Money financialImpact() const
        {
            return financialImpact_;
        }

        void setType(
            EvidenceType type)
        {
            type_ = type;
        }

        void setStrength(
            EvidenceStrength strength)
        {
            strength_ = strength;
        }

        void setSourceId(
            std::string sourceId)
        {
            sourceId_ =
                std::move(sourceId);
        }

        void setDescription(
            std::string description)
        {
            description_ =
                std::move(description);
        }

        void setFinancialImpact(
            Money financialImpact)
        {
            financialImpact_ =
                financialImpact;
        }

    private:

        std::string id_;

        EvidenceType type_ =
            EvidenceType::ReconciliationFinding;

        EvidenceStrength strength_ =
            EvidenceStrength::Weak;

        std::string sourceId_;

        std::string description_;

        Money financialImpact_{0};
    };

}