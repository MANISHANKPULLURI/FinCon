#pragma once

#include "application/investigation/FinancialDataRepository.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace fincon
{

    class InMemoryFinancialDataRepository final
        : public FinancialDataRepository
    {
    public:
        InMemoryFinancialDataRepository(
            const std::vector<Payment>& payments,
            const std::vector<Settlement>& settlements,
            const std::vector<Refund>& refunds,
            const std::vector<BankTransaction>& bankTransactions,
            const std::vector<AccountingEntry>& accountingEntries,
            const std::vector<Fee>& fees
        );

        const Payment* getPayment(
            const std::string& paymentId
        ) const override;

        const Settlement* getSettlement(
            const std::string& settlementId
        ) const override;

        const Refund* getRefund(
            const std::string& refundId
        ) const override;

        const BankTransaction* getBankTransaction(
            const std::string& bankTransactionId
        ) const override;

        const AccountingEntry* getAccountingEntry(
            const std::string& accountingEntryId
        ) const override;

        const Fee* getFee(
            const std::string& feeId
        ) const override;

        std::vector<const Refund*> getRefunds(
            const std::string& paymentId
        ) const override;

        std::vector<const BankTransaction*> getBankTransactions(
            const std::string& settlementId
        ) const override;

        std::vector<const AccountingEntry*> getAccountingEntries(
            const std::string& settlementId
        ) const override;

        std::vector<const Fee*> getFees(
            const std::string& paymentId
        ) const override;

    private:
        std::unordered_map<std::string, const Payment*> payments_;
        std::unordered_map<std::string, const Settlement*> settlements_;
        std::unordered_map<std::string, const Refund*> refunds_;
        std::unordered_map<std::string, const BankTransaction*> bankTransactions_;
        std::unordered_map<std::string, const AccountingEntry*> accountingEntries_;
        std::unordered_map<std::string, const Fee*> fees_;

        std::unordered_map<
            std::string,
            std::vector<const Refund*>
        > refundsByPayment_;

        std::unordered_map<
            std::string,
            std::vector<const BankTransaction*>
        > bankTransactionsBySettlement_;

        std::unordered_map<
            std::string,
            std::vector<const AccountingEntry*>
        > accountingEntriesBySettlement_;

        std::unordered_map<
            std::string,
            std::vector<const Fee*>
        > feesByPayment_;
    };

}