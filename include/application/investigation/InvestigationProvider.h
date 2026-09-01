#pragma once

#include "domain/investigation/InvestigationEvidence.h"

#include <string>
#include <vector>

namespace fincon
{

    class EvidenceProvider
    {
    public:
        virtual ~EvidenceProvider() = default;

        virtual std::vector<InvestigationEvidence> collect(
            const std::vector<std::string>& entityIds
        ) const = 0;
    };

}