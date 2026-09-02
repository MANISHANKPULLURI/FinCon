#pragma once

#include "domain/investigation/InvestigationToolRequest.h"

#include <string>

namespace fincon
{
    class InvestigationToolRequestValidator
    {
    public:
        virtual ~InvestigationToolRequestValidator() = default;

        virtual bool validate(
            const InvestigationToolRequest& request,
            std::string& error
        ) const = 0;
    };
}