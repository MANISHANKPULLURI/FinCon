#include "application/investigation/InvestigationToolRegistry.h"

#include <utility>

namespace fincon
{

    void InvestigationToolRegistry::registerTool(
        std::string toolName,
        std::unique_ptr<InvestigationTool> tool)
    {
        tools_[std::move(toolName)] = std::move(tool);
    }

    const InvestigationTool*
    InvestigationToolRegistry::get(
        const std::string& toolName) const
    {
        const auto iterator = tools_.find(toolName);

        if (iterator == tools_.end())
        {
            return nullptr;
        }

        return iterator->second.get();
    }

}