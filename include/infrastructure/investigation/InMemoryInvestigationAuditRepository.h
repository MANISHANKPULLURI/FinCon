#pragma once

#include "application/investigation/InvestigationAuditRepository.h"

#include <vector>

namespace fincon
{
    class InMemoryInvestigationAuditRepository final
        : public InvestigationAuditRepository
    {
    public:
        void append(
            const InvestigationAuditEntry& entry
        ) override;

        std::vector<InvestigationAuditEntry> getByInvestigation(
            const std::string& investigationId
        ) const override;

    private:
        std::vector<InvestigationAuditEntry> entries_;
    };
}