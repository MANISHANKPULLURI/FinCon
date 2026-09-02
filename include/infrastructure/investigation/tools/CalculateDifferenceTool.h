#pragma once

#include "application/investigation/FinancialDataRepository.h"
#include "application/investigation/InvestigationTool.h"

namespace fincon
{
    class CalculateDifferenceTool final
        : public InvestigationTool
    {
    public:
        explicit CalculateDifferenceTool(
            const FinancialDataRepository& repository
        );

        InvestigationToolCall execute(
            const InvestigationToolCall& request
        ) const override;

    private:
        const FinancialDataRepository& repository_;
    };
}