#pragma once

#include "application/investigation/InvestigationTool.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace fincon
{

    class InvestigationToolRegistry final
    {
    public:
        void registerTool(
            std::string toolName,
            std::unique_ptr<InvestigationTool> tool
        );

        const InvestigationTool* get(
            const std::string& toolName
        ) const;

    private:
        std::unordered_map<
            std::string,
            std::unique_ptr<InvestigationTool>
        > tools_;
    };

}