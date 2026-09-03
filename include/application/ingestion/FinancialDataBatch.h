#pragma once

#include "infrastructure/generator/FinancialDataGenerator.h"

#include <string>

namespace fincon
{
    struct FinancialDataBatch
    {
        std::string batchId;
        FinancialDataset data;
    };
}