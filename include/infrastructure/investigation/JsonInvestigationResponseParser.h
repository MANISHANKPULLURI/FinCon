#pragma once

#include "application/investigation/InvestigationResponseParser.h"

namespace fincon
{
    class JsonInvestigationResponseParser final
        : public InvestigationResponseParser
    {
    public:
        InvestigationResponse parse(
            const std::string& response
        ) const override;
    };
}