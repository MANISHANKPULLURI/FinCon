#pragma once

#include "application/investigation/EvidenceProvider.h"
#include "application/investigation/FinancialDataRepository.h"

#include <string>
#include <vector>

namespace fincon
{

    class DeterministicEvidenceProvider final
        : public EvidenceProvider
    {
    public:
        explicit DeterministicEvidenceProvider(
            const FinancialDataRepository& repository
        );

        std::vector<InvestigationEvidence> collect(
            const std::vector<std::string>& entityIds
        ) const override;

    private:
        const FinancialDataRepository& repository_;
    };

}