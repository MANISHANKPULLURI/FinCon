#pragma once

#include "infrastructure/generator/FinancialDataGenerator.h"

#include <cstdint>
#include <string>
#include <vector>

namespace fincon {

enum class ExceptionType {
    DelayedSettlement,
    Duplicate,
    PartialMatch,
    RefundMismatch,
    DateShift,
    FeeDiscrepancy,
    MissingRecord,
    Unresolvable
};

enum class EntityType {
    Payment,
    Refund,
    Fee,
    Settlement,
    BankTransaction,
    AccountingEntry
};

struct InjectedException {
    std::string id;
    ExceptionType type;

    std::vector<EntityType> entities;
    std::vector<std::string> entityIds;

    std::string expectedValue;
    std::string observedValue;

    std::int64_t financialImpact{0};

    std::string reason;
};

enum class ExceptionInjectionMode {
    Random,
    Coverage
};

class ExceptionInjector {
public:
    explicit ExceptionInjector(
        std::uint64_t seed,
        ExceptionInjectionMode mode =
            ExceptionInjectionMode::Random
    );

    std::vector<InjectedException> inject(
        FinancialDataset& dataset,
        std::uint32_t exceptionRatePercent
    ) const;

private:
    std::uint64_t seed_;
    ExceptionInjectionMode mode_;
};

}