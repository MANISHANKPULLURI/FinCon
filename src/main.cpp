#include "infrastructure/exception/ExceptionInjector.h"
#include "infrastructure/generator/FinancialDataGenerator.h"

#include <cstdint>
#include <iostream>

int main() {
    constexpr std::uint64_t seed = 42;
    constexpr std::uint32_t exceptionRatePercent = 30;

    fincon::FinancialDataGenerator generator(seed);

    fincon::FinancialDataset dataset =
        generator.generate(2, 10);

    fincon::ExceptionInjector injector(seed);

    const std::vector<fincon::InjectedException> exceptions =
        injector.inject(
            dataset,
            exceptionRatePercent
        );

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

    for (const auto& exception : exceptions) {
        std::cout << exception.id
                  << " | "
                  << exception.reason
                  << " | entities=";

        for (std::size_t index = 0;
             index < exception.entityIds.size();
             ++index) {

            if (index > 0) {
                std::cout << ", ";
            }

            std::cout << exception.entityIds[index];
        }

        std::cout << " | impact="
                  << exception.financialImpact
                  << '\n';
    }

    return 0;
}