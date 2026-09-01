#pragma once

#include "common/Money.h"
#include "common/Timestamp.h"

#include <string>
#include <vector>

namespace fincon {

enum class FindingType {
    AmountMismatch,
    MissingRecord,
    DuplicateRecord,
    DateMismatch,
    FeeDiscrepancy,
    RefundMismatch,
    DelayedSettlement,
    Unresolvable
};

enum class FindingSeverity {
    Low,
    Medium,
    High,
    Critical
};

struct ReconciliationFinding {
    std::string id;

    FindingType type;
    FindingSeverity severity;

    std::vector<std::string> entityIds;

    std::string expectedValue;
    std::string observedValue;

    Money financialImpact{0};

    std::string ruleId;
    std::string description;

    Timestamp detectedAt;
};

}