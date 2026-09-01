#include "infrastructure/exception/ExceptionInjector.h"

#include <chrono>
#include <random>
#include <string>
#include <vector>

namespace fincon
{

    namespace
    {

        std::string exceptionTypeName(ExceptionType type)
        {
            switch (type)
            {
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
            std::int64_t second)
        {
            return first >= second
                       ? first - second
                       : second - first;
        }

        Payment *findPayment(
            FinancialDataset &dataset,
            const std::string &paymentId)
        {
            for (Payment &payment : dataset.payments)
            {
                if (payment.id == paymentId)
                {
                    return &payment;
                }
            }

            return nullptr;
        }

        Refund *findRefundForPayment(
            FinancialDataset &dataset,
            const std::string &paymentId)
        {
            for (Refund &refund : dataset.refunds)
            {
                if (refund.paymentId == paymentId)
                {
                    return &refund;
                }
            }

            return nullptr;
        }

        Fee *findFeeForPayment(
            FinancialDataset &dataset,
            const std::string &paymentId)
        {
            for (Fee &fee : dataset.fees)
            {
                if (fee.paymentId == paymentId)
                {
                    return &fee;
                }
            }

            return nullptr;
        }

        BankTransaction *findBankTransaction(
            FinancialDataset &dataset,
            const Settlement &settlement)
        {
            for (BankTransaction &transaction :
                 dataset.bankTransactions)
            {
                if (transaction.settlementId ==
                    settlement.id)
                {
                    return &transaction;
                }
            }

            return nullptr;
        }

        AccountingEntry *findAccountingEntry(
            FinancialDataset &dataset,
            const Settlement &settlement)
        {
            for (AccountingEntry &entry :
                 dataset.accountingEntries)
            {
                if (entry.reference == settlement.id)
                {
                    return &entry;
                }
            }

            return nullptr;
        }

        void addEntity(
            InjectedException &exception,
            EntityType entity,
            const std::string &entityId)
        {
            exception.entities.push_back(entity);
            exception.entityIds.push_back(entityId);
        }

        ExceptionType nextCoverageType(
            std::size_t exceptionCount)
        {
            return static_cast<ExceptionType>(
                exceptionCount %
                static_cast<std::size_t>(8));
        }

    }

    ExceptionInjector::ExceptionInjector(
        std::uint64_t seed,
        ExceptionInjectionMode mode)
        : seed_(seed),
          mode_(mode)
    {
    }

    std::vector<InjectedException> ExceptionInjector::inject(
        FinancialDataset &dataset,
        std::uint32_t exceptionRatePercent) const
    {
        std::vector<InjectedException> exceptions;

        if (dataset.settlements.empty() ||
            exceptionRatePercent == 0)
        {
            return exceptions;
        }

        std::mt19937_64 generator(seed_);

        std::bernoulli_distribution shouldInject(
            static_cast<double>(exceptionRatePercent) / 100.0);

        std::uniform_int_distribution<int> typeDistribution(0, 7);

        std::size_t exceptionCounter = 1;

        const std::size_t originalSettlementCount =
            dataset.settlements.size();

        for (std::size_t index = 0;
             index < originalSettlementCount &&
             (mode_ == ExceptionInjectionMode::Random ||
              exceptions.size() < 8);
             ++index)
        {
            if (mode_ == ExceptionInjectionMode::Random &&
                !shouldInject(generator))
            {
                continue;
            }

            if (index >= dataset.settlements.size())
            {
                break;
            }

            Settlement &settlement =
                dataset.settlements[index];

            if (settlement.paymentIds.empty())
            {
                continue;
            }

            const std::string paymentId =
                settlement.paymentIds.front();

            Payment *payment =
                findPayment(dataset, paymentId);

            ExceptionType type;

            if (mode_ == ExceptionInjectionMode::Coverage)
            {
                type = nextCoverageType(exceptions.size());
            }
            else
            {
                type = static_cast<ExceptionType>(
                    typeDistribution(generator));
            }

            if (type == ExceptionType::RefundMismatch &&
                (payment == nullptr ||
                 findRefundForPayment(
                     dataset,
                     payment->id) == nullptr))
            {
                continue;
            }

            if (type == ExceptionType::FeeDiscrepancy &&
                (payment == nullptr ||
                 findFeeForPayment(
                     dataset,
                     payment->id) == nullptr))
            {
                continue;
            }

            InjectedException exception;

            exception.id =
                "EX-" +
                std::to_string(exceptionCounter++);

            exception.type = type;
            exception.reason = exceptionTypeName(type);

            switch (type)
            {

            case ExceptionType::DelayedSettlement:
            {
                addEntity(
                    exception,
                    EntityType::Settlement,
                    settlement.id);

                settlement.settledAt +=
                    std::chrono::hours(48);

                exception.expectedValue =
                    "original settlement timestamp";

                exception.observedValue =
                    "settlement delayed by 48 hours";

                exception.financialImpact = 0;

                break;
            }

            case ExceptionType::Duplicate:
            {
                Settlement duplicatedSettlement =
                    settlement;

                duplicatedSettlement.id =
                    settlement.id + "-DUP";

                dataset.settlements.push_back(
                    duplicatedSettlement);

                addEntity(
                    exception,
                    EntityType::Settlement,
                    settlement.id);

                addEntity(
                    exception,
                    EntityType::Settlement,
                    duplicatedSettlement.id);

                exception.expectedValue =
                    "one settlement";

                exception.observedValue =
                    "duplicate settlement";

                exception.financialImpact =
                    settlement.netAmount;

                break;
            }

            case ExceptionType::PartialMatch:
            {
                const Money expected =
                    settlement.netAmount;

                const Money observed =
                    expected / 2;

                settlement.netAmount =
                    observed;

                addEntity(
                    exception,
                    EntityType::Settlement,
                    settlement.id);

                if (payment != nullptr)
                {
                    addEntity(
                        exception,
                        EntityType::Payment,
                        payment->id);
                }

                exception.expectedValue =
                    std::to_string(expected);

                exception.observedValue =
                    std::to_string(observed);

                exception.financialImpact =
                    absoluteDifference(
                        expected,
                        observed);

                break;
            }

            case ExceptionType::RefundMismatch:
            {
                Refund *refund =
                    findRefundForPayment(
                        dataset,
                        payment->id);

                const Money expected =
                    refund->amount;

                const Money observed =
                    expected + 1000;

                refund->amount =
                    observed;

                addEntity(
                    exception,
                    EntityType::Refund,
                    refund->id);

                addEntity(
                    exception,
                    EntityType::Payment,
                    payment->id);

                addEntity(
                    exception,
                    EntityType::Settlement,
                    settlement.id);

                exception.expectedValue =
                    std::to_string(expected);

                exception.observedValue =
                    std::to_string(observed);

                exception.financialImpact =
                    absoluteDifference(
                        expected,
                        observed);

                break;
            }

            case ExceptionType::DateShift:
            {
                addEntity(
                    exception,
                    EntityType::Settlement,
                    settlement.id);

                BankTransaction *transaction =
                    findBankTransaction(
                        dataset,
                        settlement);

                if (transaction == nullptr)
                {
                    continue;
                }

                transaction->postedAt +=
                    std::chrono::hours(24);

                addEntity(
                    exception,
                    EntityType::BankTransaction,
                    transaction->id);

                exception.expectedValue =
                    "bank posting aligned with settlement";

                exception.observedValue =
                    "bank posting shifted by 24 hours";

                exception.financialImpact = 0;

                break;
            }

            case ExceptionType::FeeDiscrepancy:
            {
                Fee *fee =
                    findFeeForPayment(
                        dataset,
                        payment->id);

                const Money expected =
                    fee->amount;

                const Money observed =
                    expected + 500;

                fee->amount =
                    observed;

                addEntity(
                    exception,
                    EntityType::Fee,
                    fee->id);

                addEntity(
                    exception,
                    EntityType::Payment,
                    payment->id);

                addEntity(
                    exception,
                    EntityType::Settlement,
                    settlement.id);

                if (BankTransaction *transaction =
                        findBankTransaction(
                            dataset,
                            settlement))
                {
                    addEntity(
                        exception,
                        EntityType::BankTransaction,
                        transaction->id);
                }

                if (AccountingEntry *entry =
                        findAccountingEntry(
                            dataset,
                            settlement))
                {
                    addEntity(
                        exception,
                        EntityType::AccountingEntry,
                        entry->id);
                }

                exception.expectedValue =
                    std::to_string(expected);

                exception.observedValue =
                    std::to_string(observed);

                exception.financialImpact =
                    absoluteDifference(
                        expected,
                        observed);

                break;
            }
            case ExceptionType::MissingRecord:
            {
                const std::string settlementId =
                    settlement.id;

                addEntity(
                    exception,
                    EntityType::Settlement,
                    settlementId);

                exception.expectedValue =
                    "settlement exists";

                exception.observedValue =
                    "settlement missing";

                if (payment != nullptr)
                {
                    exception.financialImpact =
                        payment->amount;
                }
                else
                {
                    exception.financialImpact =
                        settlement.netAmount;
                }

                dataset.settlements.erase(
                    dataset.settlements.begin() +
                    static_cast<std::ptrdiff_t>(index));

                if (index > 0)
                {
                    --index;
                }

                break;
            }

            case ExceptionType::Unresolvable:
            {
                const Money expected =
                    settlement.netAmount;

                const Money observed =
                    expected + 1;

                settlement.netAmount =
                    observed;

                addEntity(
                    exception,
                    EntityType::Settlement,
                    settlement.id);

                if (BankTransaction *transaction =
                        findBankTransaction(
                            dataset,
                            settlement))
                {
                    addEntity(
                        exception,
                        EntityType::BankTransaction,
                        transaction->id);
                }

                exception.expectedValue =
                    std::to_string(expected);

                exception.observedValue =
                    std::to_string(observed);

                exception.financialImpact = 1;

                break;
            }
            }

            exceptions.push_back(exception);
        }

        return exceptions;
    }

}