#pragma once

#include "application/investigation/FinancialDataRepository.h"
#include "application/investigation/InvestigationTool.h"

namespace fincon
{
    class GetAccountingEntriesTool final
        : public InvestigationTool
    {
    public:
        explicit GetAccountingEntriesTool(
            const FinancialDataRepository& repository
        );

        InvestigationToolCall execute(
            const InvestigationToolCall& request
        ) const override;

    private:
        const FinancialDataRepository& repository_;
    };
}