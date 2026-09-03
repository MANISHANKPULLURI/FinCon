#pragma once

#include "common/Money.h"

#include <string>
#include <vector>

namespace fincon
{

    struct FindingCorrelation
    {
        std::string id;

        std::vector<std::string> findingIds;

        std::vector<std::string> entityIds;

        std::vector<std::string> ruleIds;

        Money financialExposure{0};
    };

}