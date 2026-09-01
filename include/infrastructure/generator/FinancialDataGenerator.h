#pragma once

#include "domain/financial/AccountingEntry.h"
#include "domain/financial/BankTransaction.h"
#include "domain/financial/Fee.h"
#include "domain/financial/Merchant.h"
#include "domain/financial/Order.h"
#include "domain/financial/Payment.h"
#include "domain/financial/Refund.h"
#include "domain/financial/Settlement.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace fincon {

struct FinancialDataset {
    std::vector<Merchant> merchants;
    std::vector<Order> orders;
    std::vector<Payment> payments;
    std::vector<Refund> refunds;
    std::vector<Fee> fees;
    std::vector<Settlement> settlements;
    std::vector<BankTransaction> bankTransactions;
    std::vector<AccountingEntry> accountingEntries;
};

class FinancialDataGenerator {
public:
    explicit FinancialDataGenerator(std::uint64_t seed);

    FinancialDataset generate(
        std::size_t merchantCount,
        std::size_t ordersPerMerchant
    ) const;

private:
    std::uint64_t seed_;
};

} 