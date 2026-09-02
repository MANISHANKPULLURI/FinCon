#pragma once

#include "application/investigation/InvestigationAuditRepository.h"
#include "domain/investigation/InvestigationAuditEntry.h"

#include <cstdint>
#include <string>

namespace fincon
{
    class InvestigationAuditService
    {
    public:
        explicit InvestigationAuditService(
            InvestigationAuditRepository& repository
        );

        void record(
            const std::string& investigationId,
            const std::string& incidentId,
            AuditEventType eventType,
            const std::string& actor,
            const std::string& description,
            std::int64_t financialImpact = 0
        );

    private:
        InvestigationAuditRepository& repository_;
        std::uint64_t sequence_ = 0;
    };
}