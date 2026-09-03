#include "application/controller/FinanceControllerFacade.h"

namespace fincon
{
    FinanceControllerFacade::FinanceControllerFacade(
        InvestigationService& investigationService,
        InvestigationAuditRepository& auditRepository
    )
        : investigationService_(investigationService),
          auditRepository_(auditRepository)
    {
    }

    Investigation FinanceControllerFacade::investigate(
        const Incident& incident
    ) const
    {
        return investigationService_.investigate(
            incident
        );
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
}