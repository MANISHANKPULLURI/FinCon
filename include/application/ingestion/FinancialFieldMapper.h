#pragma once

#include "application/ingestion/FinancialDataBatch.h"

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace fincon
{
    struct MappingResult
    {
        bool accepted = false;
        std::string batchId;
        std::string status;
        std::string reason;
        std::vector<std::string> missingFields;
        FinancialDataBatch batch;
    };

    class FinancialFieldMapper final
    {
    public:
        MappingResult map(const nlohmann::json& payload) const;
    };
}