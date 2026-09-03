#include "application/controller/FinanceControllerFacade.h"

namespace fincon
{
    FinanceControllerFacade::FinanceControllerFacade(
        InvestigationService& investigationService,
                InvestigationAuditRepository& auditRepository,
                FinanceControllerState& state
    )
        : investigationService_(investigationService),
                    auditRepository_(auditRepository),
                    state_(state)
    {
    }

    Investigation FinanceControllerFacade::investigate(
        const Incident& incident
    ) const
    {
        Investigation investigation =
            investigationService_.investigate(incident);
        state_.setInvestigation(investigation);
        return investigation;
    }

    std::vector<InvestigationAuditEntry>
    FinanceControllerFacade::getAuditTrail(
        const std::string& investigationId
    ) const
    {
        return auditRepository_.getByInvestigation(
            investigationId
        );
    }

    FinanceControllerState::Snapshot
    FinanceControllerFacade::snapshot() const
    {
        return state_.snapshot();
    }
}