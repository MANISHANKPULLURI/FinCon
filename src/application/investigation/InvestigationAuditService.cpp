#include "application/investigation/InvestigationAuditService.h"

namespace fincon
{
    InvestigationAuditService::InvestigationAuditService(
        InvestigationAuditRepository& repository)
        : repository_(repository)
    {
    }

    void InvestigationAuditService::record(
        const std::string& investigationId,
        const std::string& incidentId,
        AuditEventType eventType,
        const std::string& actor,
        const std::string& description,
        std::int64_t financialImpact)
    {
        InvestigationAuditEntry entry(
            "AUDIT-" + std::to_string(++sequence_)
        );

        entry.setInvestigationId(investigationId);
        entry.setIncidentId(incidentId);
        entry.setEventType(eventType);
        entry.setActor(actor);
        entry.setDescription(description);
        entry.setFinancialImpact(financialImpact);

        repository_.append(entry);
    }
}