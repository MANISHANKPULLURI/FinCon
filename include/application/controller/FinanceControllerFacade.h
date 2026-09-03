#pragma once

#include "application/investigation/InvestigationAuditRepository.h"
#include "application/investigation/InvestigationService.h"
#include "application/state/FinanceControllerState.h"
#include "domain/incident/Incident.h"
#include "domain/investigation/Investigation.h"

#include <cstddef>
#include <vector>

namespace fincon
{
    class FinanceControllerFacade
    {
    public:
        FinanceControllerFacade(
            InvestigationService& investigationService,
            InvestigationAuditRepository& auditRepository,
            FinanceControllerState& state
        );

        Investigation investigate(
            const Incident& incident
        ) const;

        std::vector<InvestigationAuditEntry> getAuditTrail(
            const std::string& investigationId
        ) const;

        FinanceControllerState::Snapshot snapshot() const;

    private:
        InvestigationService& investigationService_;
        InvestigationAuditRepository& auditRepository_;
        FinanceControllerState& state_;
    };
}