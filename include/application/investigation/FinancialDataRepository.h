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

        virtual const Payment* getPayment(
            const std::string& paymentId
        ) const = 0;

        virtual const Settlement* getSettlement(
            const std::string& settlementId
        ) const = 0;

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
            const std::string& settlementId
        ) const = 0;
    };

}