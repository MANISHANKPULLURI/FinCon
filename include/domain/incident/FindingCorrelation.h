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

        Money financialExposure{0};
    };

}