#pragma once

#include "domain/financial/AccountingEntry.h"
#include "domain/financial/BankTransaction.h"
#include "domain/financial/Fee.h"
#include "domain/financial/Payment.h"
#include "domain/financial/Refund.h"
#include "domain/financial/Settlement.h"

#include <string>
#include <vector>

namespace fincon
{

    class FinancialDataRepository
    {
    public:
        virtual ~FinancialDataRepository() = default;

        // Direct entity lookups
        virtual const Payment* getPayment(
            const std::string& paymentId
        ) const = 0;

        virtual const Settlement* getSettlement(
            const std::string& settlementId
        ) const = 0;

        virtual const Refund* getRefund(
            const std::string& refundId
        ) const = 0;

        virtual const BankTransaction* getBankTransaction(
            const std::string& bankTransactionId
        ) const = 0;

        virtual const AccountingEntry* getAccountingEntry(
            const std::string& accountingEntryId
        ) const = 0;

        virtual const Fee* getFee(
            const std::string& feeId
        ) const = 0;

        // Relationship lookups
        virtual std::vector<const Refund*> getRefunds(
            const std::string& paymentId
        ) const = 0;

        virtual std::vector<const BankTransaction*> getBankTransactions(
            const std::string& settlementId
        ) const = 0;

        virtual std::vector<const AccountingEntry*> getAccountingEntries(
            const std::string& settlementId
        ) const = 0;

        virtual std::vector<const Fee*> getFees(
            const std::string& paymentId
        ) const = 0;
    };

}