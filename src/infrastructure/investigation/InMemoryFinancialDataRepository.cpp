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
        FinancialDataset initial;
        initial.payments = payments;
        initial.settlements = settlements;
        initial.refunds = refunds;
        initial.bankTransactions = bankTransactions;
        initial.accountingEntries = accountingEntries;
        initial.fees = fees;
        appendBatch(initial);
    }

    void InMemoryFinancialDataRepository::appendBatch(
        const FinancialDataset& batch)
    {
        std::unique_lock lock(mutex_);
        for (const Payment& payment : batch.payments)
        {
            paymentStorage_.push_back(payment);
            payments_[paymentStorage_.back().id] = &paymentStorage_.back();
        }
        for (const Settlement& settlement : batch.settlements)
        {
            settlementStorage_.push_back(settlement);
            settlements_[settlementStorage_.back().id] = &settlementStorage_.back();
        }
        for (const Refund& refund : batch.refunds)
        {
            refundStorage_.push_back(refund);
            refunds_[refundStorage_.back().id] = &refundStorage_.back();
            refundsByPayment_[refundStorage_.back().paymentId].push_back(&refundStorage_.back());
        }
        for (const BankTransaction& transaction : batch.bankTransactions)
        {
            bankTransactionStorage_.push_back(transaction);
            bankTransactions_[bankTransactionStorage_.back().id] = &bankTransactionStorage_.back();
            bankTransactionsBySettlement_[bankTransactionStorage_.back().settlementId].push_back(&bankTransactionStorage_.back());
        }
        for (const AccountingEntry& entry : batch.accountingEntries)
        {
            accountingEntryStorage_.push_back(entry);
            accountingEntries_[accountingEntryStorage_.back().id] = &accountingEntryStorage_.back();
            accountingEntriesBySettlement_[accountingEntryStorage_.back().reference].push_back(&accountingEntryStorage_.back());
        }
        for (const Fee& fee : batch.fees)
        {
            feeStorage_.push_back(fee);
            fees_[feeStorage_.back().id] = &feeStorage_.back();
            feesByPayment_[feeStorage_.back().paymentId].push_back(&feeStorage_.back());
        }
    }

    const Payment*
    InMemoryFinancialDataRepository::getPayment(
        const std::string& paymentId) const
    {
        std::shared_lock lock(mutex_);
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
        std::shared_lock lock(mutex_);
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
        std::shared_lock lock(mutex_);
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
        std::shared_lock lock(mutex_);
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
        std::shared_lock lock(mutex_);
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
        std::shared_lock lock(mutex_);
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
        std::shared_lock lock(mutex_);
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
        std::shared_lock lock(mutex_);
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
        std::shared_lock lock(mutex_);
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
        std::shared_lock lock(mutex_);
        const auto iterator =
            feesByPayment_.find(paymentId);

        if (iterator == feesByPayment_.end())
        {
            return {};
        }

        return iterator->second;
    }

}