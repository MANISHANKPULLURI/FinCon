#pragma once

#include "domain/investigation/InvestigationRequest.h"
#include "domain/investigation/InvestigationResponse.h"

#include <string>

namespace fincon
{
    class InvestigationResponseValidator
    {
    public:
        virtual ~InvestigationResponseValidator() = default;

        virtual bool validate(
            const InvestigationResponse& response,
            const InvestigationRequest& request,
            std::string& error
        ) const = 0;
    };
}