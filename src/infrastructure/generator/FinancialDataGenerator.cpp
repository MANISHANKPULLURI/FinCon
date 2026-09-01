#include "infrastructure/generator/FinancialDataGenerator.h"

#include <chrono>
#include <random>
#include <string>

namespace fincon {

FinancialDataGenerator::FinancialDataGenerator(std::uint64_t seed)
    : seed_(seed) {}

FinancialDataset FinancialDataGenerator::generate(
    std::size_t merchantCount,
    std::size_t ordersPerMerchant) const {

    FinancialDataset dataset;

    std::mt19937_64 generator(seed_);

    std::uniform_int_distribution<std::int64_t> amountDistribution(
        10000,
        5000000
    );

    std::uniform_int_distribution<int> refundChance(1, 100);
    std::uniform_int_distribution<int> feeChance(1, 100);

    const Timestamp baseTime =
        Timestamp{
            std::chrono::system_clock::from_time_t(0)
        };

    for (std::size_t merchantIndex = 0;
         merchantIndex < merchantCount;
         ++merchantIndex) {

        Merchant merchant;
        merchant.id =
            "M-" + std::to_string(merchantIndex + 1);

        merchant.name =
            "Merchant-" +
            std::to_string(merchantIndex + 1);

        dataset.merchants.push_back(merchant);

        for (std::size_t orderIndex = 0;
             orderIndex < ordersPerMerchant;
             ++orderIndex) {

            const std::string suffix =
                std::to_string(merchantIndex + 1) +
                "-" +
                std::to_string(orderIndex + 1);

            const Money paymentAmount =
                amountDistribution(generator);

            const std::size_t transactionIndex =
                merchantIndex * ordersPerMerchant +
                orderIndex;

            const Timestamp createdAt =
                baseTime +
                std::chrono::hours(
                    static_cast<int>(transactionIndex)
                );

            Order order;
            order.id = "O-" + suffix;
            order.merchantId = merchant.id;
            order.amount = paymentAmount;
            order.status = OrderStatus::Paid;
            order.createdAt = createdAt;

            dataset.orders.push_back(order);

            Payment payment;
            payment.id = "P-" + suffix;
            payment.merchantId = merchant.id;
            payment.orderId = order.id;
            payment.amount = paymentAmount;
            payment.status = PaymentStatus::Captured;
            payment.createdAt =
                createdAt +
                std::chrono::minutes(2);

            dataset.payments.push_back(payment);

            Money refundAmount = 0;

            if (refundChance(generator) <= 15) {
                refundAmount =
                    paymentAmount / 10;

                Refund refund;
                refund.id = "R-" + suffix;
                refund.merchantId = merchant.id;
                refund.paymentId = payment.id;
                refund.amount = refundAmount;
                refund.status = RefundStatus::Processed;
                refund.createdAt =
                    payment.createdAt +
                    std::chrono::hours(6);
                refund.processedAt =
                    refund.createdAt +
                    std::chrono::minutes(10);

                dataset.refunds.push_back(refund);
            }

            Money feeAmount = 0;

            if (feeChance(generator) <= 80) {
                feeAmount =
                    paymentAmount / 50;

                Fee fee;
                fee.id = "F-" + suffix;
                fee.merchantId = merchant.id;
                fee.paymentId = payment.id;
                fee.type = FeeType::Processing;
                fee.amount = feeAmount;
                fee.createdAt =
                    payment.createdAt +
                    std::chrono::minutes(5);

                dataset.fees.push_back(fee);
            }

            const Money settlementAmount =
                paymentAmount -
                refundAmount -
                feeAmount;

            Settlement settlement;
            settlement.id = "S-" + suffix;
            settlement.merchantId = merchant.id;
            settlement.paymentIds.push_back(payment.id);
            settlement.grossAmount = paymentAmount;
            settlement.refundAmount = refundAmount;
            settlement.feeAmount = feeAmount;
            settlement.netAmount = settlementAmount;
            settlement.status = SettlementStatus::Settled;
            settlement.createdAt =
                payment.createdAt +
                std::chrono::hours(24);
            settlement.settledAt =
                settlement.createdAt +
                std::chrono::hours(24);

            dataset.settlements.push_back(settlement);

            BankTransaction bankTransaction;
            bankTransaction.id = "B-" + suffix;
            bankTransaction.merchantId = merchant.id;
            bankTransaction.amount = settlementAmount;
            bankTransaction.type =
                BankTransactionType::Credit;
            bankTransaction.postedAt =
                settlement.settledAt +
                std::chrono::hours(1);

            dataset.bankTransactions.push_back(
                bankTransaction
            );

            AccountingEntry accountingEntry;
            accountingEntry.id = "A-" + suffix;
            accountingEntry.merchantId = merchant.id;
            accountingEntry.reference =
                settlement.id;
            accountingEntry.amount =
                settlementAmount;
            accountingEntry.type =
                AccountingEntryType::Credit;
            accountingEntry.createdAt =
                settlement.settledAt +
                std::chrono::hours(2);

            dataset.accountingEntries.push_back(
                accountingEntry
            );
        }
    }

    return dataset;
}

}