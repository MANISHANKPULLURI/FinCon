#pragma once

#include "domain/investigation/InvestigationAuditEntry.h"

#include <string>
#include <vector>

namespace fincon
{
    class InvestigationAuditRepository
    {
    public:
        virtual ~InvestigationAuditRepository() = default;

        virtual void append(
            const InvestigationAuditEntry& entry
        ) = 0;

        virtual std::vector<InvestigationAuditEntry> getByInvestigation(
            const std::string& investigationId
        ) const = 0;
    };
}
