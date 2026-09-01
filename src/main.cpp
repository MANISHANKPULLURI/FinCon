#include "infrastructure/exception/ExceptionInjector.h"
#include "infrastructure/generator/FinancialDataGenerator.h"
#include "infrastructure/reconciliation/ReconciliationEngine.h"
#include "infrastructure/reconciliation/rules/SettlementCalculationRule.h"
#include "infrastructure/reconciliation/rules/SettlementRefundRule.h"
#include "infrastructure/reconciliation/rules/SettlementFeeRule.h"

#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

int main()
{
    constexpr std::uint64_t seed = 42;
    constexpr std::uint32_t exceptionRatePercent = 30;

    fincon::FinancialDataGenerator generator(seed);

    fincon::FinancialDataset dataset =
        generator.generate(2, 10);

    fincon::ExceptionInjector injector(
        seed,
        fincon::ExceptionInjectionMode::Coverage
    );

    const std::vector<fincon::InjectedException> exceptions =
        injector.inject(
            dataset,
            exceptionRatePercent
        );

    fincon::ReconciliationEngine reconciliationEngine;

    reconciliationEngine.addRule(
        std::make_unique<
            fincon::SettlementCalculationRule
        >()
    );

    reconciliationEngine.addRule(
        std::make_unique<
            fincon::SettlementRefundRule
        >()
    );

    reconciliationEngine.addRule(
        std::make_unique<
            fincon::SettlementFeeRule
        >()
    );

    const std::vector<fincon::ReconciliationFinding> findings =
        reconciliationEngine.reconcile(dataset);

    std::cout << "FinCon started\n\n";

    std::cout << "Merchants: "
              << dataset.merchants.size() << '\n';

    std::cout << "Orders: "
              << dataset.orders.size() << '\n';

    std::cout << "Payments: "
              << dataset.payments.size() << '\n';

    std::cout << "Refunds: "
              << dataset.refunds.size() << '\n';

    std::cout << "Fees: "
              << dataset.fees.size() << '\n';

    std::cout << "Settlements: "
              << dataset.settlements.size() << '\n';

    std::cout << "Bank transactions: "
              << dataset.bankTransactions.size() << '\n';

    std::cout << "Accounting entries: "
              << dataset.accountingEntries.size() << '\n';

    std::cout << "\nInjected exceptions: "
              << exceptions.size() << '\n';

    for (const auto& exception : exceptions)
    {
        std::cout << exception.id
                  << " | "
                  << exception.reason
                  << " | entities=";

        for (std::size_t index = 0;
             index < exception.entityIds.size();
             ++index)
        {
            if (index > 0)
            {
                std::cout << ", ";
            }

            std::cout << exception.entityIds[index];
        }

        std::cout << " | impact="
                  << exception.financialImpact
                  << '\n';
    }

    std::cout << "\nReconciliation findings: "
              << findings.size()
              << '\n';

    for (const auto& finding : findings)
    {
        std::cout << finding.id
                  << " | "
                  << finding.ruleId
                  << " | "
                  << finding.description
                  << " | entities=";

        for (std::size_t index = 0;
             index < finding.entityIds.size();
             ++index)
        {
            if (index > 0)
            {
                std::cout << ", ";
            }

            std::cout << finding.entityIds[index];
        }

        std::cout << " | expected="
                  << finding.expectedValue
                  << " | observed="
                  << finding.observedValue
                  << " | impact="
                  << finding.financialImpact
                  << '\n';
    }

    return 0;
}