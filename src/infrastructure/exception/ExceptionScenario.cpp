#include "infrastructure/exception/ExceptionScenario.h"

namespace fincon {

const std::vector<ExceptionScenario>& exceptionScenarios() {
    static const std::vector<ExceptionScenario> scenarios{
        {
            ExceptionType::DelayedSettlement,
            EntityType::Settlement,
            "Settlement was completed later than expected"
        },
        {
            ExceptionType::Duplicate,
            EntityType::Settlement,
            "Duplicate financial record detected"
        },
        {
            ExceptionType::PartialMatch,
            EntityType::Settlement,
            "Financial records only partially match"
        },
        {
            ExceptionType::RefundMismatch,
            EntityType::Refund,
            "Refund amount differs from the expected amount"
        },
        {
            ExceptionType::DateShift,
            EntityType::Settlement,
            "Related financial records have an unexpected date shift"
        },
        {
            ExceptionType::FeeDiscrepancy,
            EntityType::Fee,
            "Fee amount differs from the expected amount"
        },
        {
            ExceptionType::MissingRecord,
            EntityType::Settlement,
            "Expected financial record is missing"
        },
        {
            ExceptionType::Unresolvable,
            EntityType::Settlement,
            "Available evidence is insufficient to determine the cause"
        }
    };

    return scenarios;
}

} 