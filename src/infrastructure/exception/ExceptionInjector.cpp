#include "infrastructure/exception/ExceptionInjector.h"

#include <chrono>
#include <random>
#include <string>

namespace fincon {

namespace {

std::string exceptionTypeName(ExceptionType type) {
    switch (type) {
        case ExceptionType::DelayedSettlement:
            return "Delayed settlement";

        case ExceptionType::Duplicate:
            return "Duplicate";

        case ExceptionType::PartialMatch:
            return "Partial match";

        case ExceptionType::RefundMismatch:
            return "Refund mismatch";

        case ExceptionType::DateShift:
            return "Date shift";

        case ExceptionType::FeeDiscrepancy:
            return "Fee discrepancy";

        case ExceptionType::MissingRecord:
            return "Missing record";

        case ExceptionType::Unresolvable:
            return "Unresolvable";
    }

    return "Unknown";
}

std::int64_t absoluteDifference(
    std::int64_t first,
    std::int64_t second) {

    return first >= second
        ? first - second
        : second - first;
}

} // namespace

ExceptionInjector::ExceptionInjector(std::uint64_t seed)
    : seed_(seed) {}

std::vector<InjectedException> ExceptionInjector::inject(
    FinancialDataset& dataset,
    std::uint32_t exceptionRatePercent) const {

    std::vector<InjectedException> exceptions;

    if (dataset.settlements.empty() ||
        exceptionRatePercent == 0) {
        return exceptions;
    }

    std::mt19937_64 generator(seed_);

    std::bernoulli_distribution shouldInject(
        static_cast<double>(exceptionRatePercent) / 100.0
    );

    std::uniform_int_distribution<int> typeDistribution(0, 7);

    std::size_t exceptionCounter = 1;

    const std::size_t originalSettlementCount =
        dataset.settlements.size();

    for (std::size_t index = 0;
         index < originalSettlementCount;
         ++index) {

        if (!shouldInject(generator)) {
            continue;
        }

        Settlement& settlement =
            dataset.settlements[index];

        const ExceptionType type =
            static_cast<ExceptionType>(
                typeDistribution(generator)
            );

        InjectedException exception;

        exception.id =
            "EX-" + std::to_string(exceptionCounter++);

        exception.type = type;
        exception.entity = EntityType::Settlement;
        exception.entityIds.push_back(settlement.id);
        exception.reason = exceptionTypeName(type);

        switch (type) {

            case ExceptionType::DelayedSettlement: {
                settlement.settledAt +=
                    std::chrono::hours(48);

                exception.expectedValue =
                    "original timestamp";

                exception.observedValue =
                    "timestamp delayed by 48 hours";

                exception.financialImpact = 0;

                break;
            }

            case ExceptionType::Duplicate: {
                Settlement duplicate = settlement;

                duplicate.id =
                    settlement.id + "-DUP";

                dataset.settlements.push_back(
                    duplicate
                );

                exception.entityIds.push_back(
                    duplicate.id
                );

                exception.expectedValue =
                    "1 settlement";

                exception.observedValue =
                    "2 settlements for the same financial event";

                exception.financialImpact =
                    settlement.netAmount;

                break;
            }

            case ExceptionType::PartialMatch: {
                const Money expected =
                    settlement.netAmount;

                const Money observed =
                    expected / 2;

                settlement.netAmount = observed;

                exception.expectedValue =
                    std::to_string(expected);

                exception.observedValue =
                    std::to_string(observed);

                exception.financialImpact =
                    absoluteDifference(
                        expected,
                        observed
                    );

                break;
            }

            case ExceptionType::RefundMismatch: {
                const Money expected =
                    settlement.refundAmount;

                const Money observed =
                    expected + 1000;

                settlement.refundAmount = observed;

                exception.expectedValue =
                    std::to_string(expected);

                exception.observedValue =
                    std::to_string(observed);

                exception.financialImpact =
                    absoluteDifference(
                        expected,
                        observed
                    );

                break;
            }

            case ExceptionType::DateShift: {
                settlement.settledAt +=
                    std::chrono::hours(24);

                exception.expectedValue =
                    "original settlement date";

                exception.observedValue =
                    "settlement date shifted by 24 hours";

                exception.financialImpact = 0;

                break;
            }

            case ExceptionType::FeeDiscrepancy: {
                const Money expected =
                    settlement.feeAmount;

                const Money observed =
                    expected + 500;

                settlement.feeAmount = observed;

                exception.expectedValue =
                    std::to_string(expected);

                exception.observedValue =
                    std::to_string(observed);

                exception.financialImpact =
                    absoluteDifference(
                        expected,
                        observed
                    );

                break;
            }

            case ExceptionType::MissingRecord: {
                exception.expectedValue =
                    "settlement exists";

                exception.observedValue =
                    "settlement missing";

                exception.financialImpact =
                    settlement.netAmount;

                dataset.settlements.erase(
                    dataset.settlements.begin() +
                    static_cast<std::ptrdiff_t>(index)
                );

                break;
            }

            case ExceptionType::Unresolvable: {
                settlement.netAmount += 1;

                exception.expectedValue =
                    "consistent settlement amount";

                exception.observedValue =
                    "ambiguous one-paise discrepancy";

                exception.financialImpact = 1;

                break;
            }
        }

        exceptions.push_back(exception);

        if (type == ExceptionType::MissingRecord) {
            --index;
        }
    }

    return exceptions;
}

} // namespace fincon