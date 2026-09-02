#include "infrastructure/investigation/InMemoryFinancialDataRepository.h"

namespace fincon
{

    InMemoryFinancialDataRepository::InMemoryFinancialDataRepository(
        const std::vector<Payment>& payments,
        const std::vector<Settlement>& settlements,
        const std::vector<Refund>& refunds,
        const std::vector<BankTransaction>& bankTransactions,
        const std::vector<AccountingEntry>& accountingEntries,
        const std::vector<Fee>& fees)
    {
        for (const Payment& payment : payments)
        {
            payments_[payment.id] = &payment;
        }

        for (const Settlement& settlement : settlements)
        {
            settlements_[settlement.id] = &settlement;
        }

        for (const Refund& refund : refunds)
        {
            refunds_[refund.id] = &refund;
            refundsByPayment_[refund.paymentId].push_back(&refund);
        }

        for (const BankTransaction& transaction : bankTransactions)
        {
            bankTransactions_[transaction.id] = &transaction;

            bankTransactionsBySettlement_[transaction.settlementId]
                .push_back(&transaction);
        }

        for (const AccountingEntry& entry : accountingEntries)
        {
            accountingEntries_[entry.id] = &entry;

            accountingEntriesBySettlement_[entry.reference]
                .push_back(&entry);
        }

        for (const Fee& fee : fees)
        {
            fees_[fee.id] = &fee;
            feesByPayment_[fee.paymentId].push_back(&fee);
        }
    }

    const Payment*
    InMemoryFinancialDataRepository::getPayment(
        const std::string& paymentId) const
    {
        const auto iterator = payments_.find(paymentId);

        if (iterator == payments_.end())
        {
            return nullptr;
        }

        return iterator->second;
    }

    const Settlement*
    InMemoryFinancialDataRepository::getSettlement(
        const std::string& settlementId) const
    {
        const auto iterator = settlements_.find(settlementId);

        if (iterator == settlements_.end())
        {
            return nullptr;
        }

        return iterator->second;
    }

    const Refund*
    InMemoryFinancialDataRepository::getRefund(
        const std::string& refundId) const
    {
        const auto iterator = refunds_.find(refundId);

        if (iterator == refunds_.end())
        {
            return nullptr;
        }

        return iterator->second;
    }

    const BankTransaction*
    InMemoryFinancialDataRepository::getBankTransaction(
        const std::string& bankTransactionId) const
    {
        const auto iterator =
            bankTransactions_.find(bankTransactionId);

        if (iterator == bankTransactions_.end())
        {
            return nullptr;
        }

        return iterator->second;
    }

    const AccountingEntry*
    InMemoryFinancialDataRepository::getAccountingEntry(
        const std::string& accountingEntryId) const
    {
        const auto iterator =
            accountingEntries_.find(accountingEntryId);

        if (iterator == accountingEntries_.end())
        {
            return nullptr;
        }

        return iterator->second;
    }

    const Fee*
    InMemoryFinancialDataRepository::getFee(
        const std::string& feeId) const
    {
        const auto iterator = fees_.find(feeId);

        if (iterator == fees_.end())
        {
            return nullptr;
        }

        return iterator->second;
    }

    std::vector<const Refund*>
    InMemoryFinancialDataRepository::getRefunds(
        const std::string& paymentId) const
    {
        const auto iterator =
            refundsByPayment_.find(paymentId);

        if (iterator == refundsByPayment_.end())
        {
            return {};
        }

        return iterator->second;
    }

    std::vector<const BankTransaction*>
    InMemoryFinancialDataRepository::getBankTransactions(
        const std::string& settlementId) const
    {
        const auto iterator =
            bankTransactionsBySettlement_.find(settlementId);

        if (iterator == bankTransactionsBySettlement_.end())
        {
            return {};
        }

        return iterator->second;
    }

    std::vector<const AccountingEntry*>
    InMemoryFinancialDataRepository::getAccountingEntries(
        const std::string& settlementId) const
    {
        const auto iterator =
            accountingEntriesBySettlement_.find(settlementId);

        if (iterator == accountingEntriesBySettlement_.end())
        {
            return {};
        }

        return iterator->second;
    }

    std::vector<const Fee*>
    InMemoryFinancialDataRepository::getFees(
        const std::string& paymentId) const
    {
        const auto iterator =
            feesByPayment_.find(paymentId);

        if (iterator == feesByPayment_.end())
        {
            return {};
        }

        return iterator->second;
    }

}