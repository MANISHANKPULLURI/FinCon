#include "infrastructure/investigation/InMemoryInvestigationAuditRepository.h"

namespace fincon
{
    void InMemoryInvestigationAuditRepository::append(
        const InvestigationAuditEntry& entry)
    {
        entries_.push_back(entry);
    }

    std::vector<InvestigationAuditEntry>
    InMemoryInvestigationAuditRepository::getByInvestigation(
        const std::string& investigationId) const
    {
        std::vector<InvestigationAuditEntry> result;

        for (const InvestigationAuditEntry& entry : entries_)
        {
            if (entry.investigationId() == investigationId)
                result.push_back(entry);
        }

        return result;
    }
}