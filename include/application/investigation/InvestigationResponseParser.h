#pragma once

#include "domain/investigation/InvestigationResponse.h"

#include <string>

namespace fincon
{
    class InvestigationResponseParser
    {
    public:
        virtual ~InvestigationResponseParser() = default;

        virtual InvestigationResponse parse(
            const std::string& response
        ) const = 0;
    };
}