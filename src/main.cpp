#include "domain/incident/FindingCorrelation.h"
#include "domain/incident/IncidentBuilder.h"

#include "infrastructure/exception/ExceptionInjector.h"
#include "infrastructure/generator/FinancialDataGenerator.h"
#include "infrastructure/reconciliation/ReconciliationEngine.h"
#include "infrastructure/reconciliation/ReconciliationFindingCorrelator.h"
#include "infrastructure/reconciliation/rules/DuplicateRecordRule.h"
#include "infrastructure/reconciliation/rules/MissingRecordRule.h"
#include "infrastructure/reconciliation/rules/PaymentSettlementMatchingRule.h"
#include "infrastructure/reconciliation/rules/SettlementAccountingRule.h"
#include "infrastructure/reconciliation/rules/SettlementBankRule.h"
#include "infrastructure/reconciliation/rules/SettlementCalculationRule.h"
#include "infrastructure/reconciliation/rules/SettlementFeeRule.h"
#include "infrastructure/reconciliation/rules/SettlementRefundRule.h"
#include "infrastructure/reconciliation/rules/SettlementTimingRule.h"

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
        std::make_unique<fincon::SettlementCalculationRule>()
    );

    reconciliationEngine.addRule(
        std::make_unique<fincon::SettlementRefundRule>()
    );

    reconciliationEngine.addRule(
        std::make_unique<fincon::SettlementFeeRule>()
    );

    reconciliationEngine.addRule(
        std::make_unique<fincon::SettlementBankRule>()
    );

    reconciliationEngine.addRule(
        std::make_unique<fincon::SettlementTimingRule>()
    );

    reconciliationEngine.addRule(
        std::make_unique<fincon::MissingRecordRule>()
    );

    reconciliationEngine.addRule(
        std::make_unique<fincon::DuplicateRecordRule>()
    );

    reconciliationEngine.addRule(
        std::make_unique<fincon::PaymentSettlementMatchingRule>()
    );

    reconciliationEngine.addRule(
        std::make_unique<fincon::SettlementAccountingRule>()
    );

    const std::vector<fincon::ReconciliationFinding> findings =
        reconciliationEngine.reconcile(dataset);

    fincon::ReconciliationFindingCorrelator correlator;

    const std::vector<fincon::FindingCorrelation> correlations =
        correlator.correlate(findings);

    fincon::IncidentBuilder incidentBuilder;

    const std::vector<fincon::Incident> incidents =
        incidentBuilder.build(correlations);

    std::cout << "FinCon started\n\n";

    std::cout << "Merchants: "
              << dataset.merchants.size()
              << '\n';

    std::cout << "Orders: "
              << dataset.orders.size()
              << '\n';

    std::cout << "Payments: "
              << dataset.payments.size()
              << '\n';

    std::cout << "Refunds: "
              << dataset.refunds.size()
              << '\n';

    std::cout << "Fees: "
              << dataset.fees.size()
              << '\n';

    std::cout << "Settlements: "
              << dataset.settlements.size()
              << '\n';

    std::cout << "Bank transactions: "
              << dataset.bankTransactions.size()
              << '\n';

    std::cout << "Accounting entries: "
              << dataset.accountingEntries.size()
              << '\n';

    std::cout << "\nInjected exceptions: "
              << exceptions.size()
              << '\n';

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

    std::cout << "\nFinding correlations: "
              << correlations.size()
              << '\n';

    for (const auto& correlation : correlations)
    {
        std::cout << correlation.id
                  << " | findings=";

        for (std::size_t index = 0;
             index < correlation.findingIds.size();
             ++index)
        {
            if (index > 0)
            {
                std::cout << ", ";
            }

            std::cout << correlation.findingIds[index];
        }

        std::cout << " | entities=";

        for (std::size_t index = 0;
             index < correlation.entityIds.size();
             ++index)
        {
            if (index > 0)
            {
                std::cout << ", ";
            }

            std::cout << correlation.entityIds[index];
        }

        std::cout << " | exposure="
                  << correlation.financialExposure
                  << '\n';
    }

    std::cout << "\nIncidents: "
              << incidents.size()
              << '\n';

    for (const auto& incident : incidents)
    {
        std::cout << incident.id()
                  << " | type="
                  << static_cast<int>(incident.type())
                  << " | status="
                  << static_cast<int>(incident.status())
                  << " | impact="
                  << incident.financialImpact()
                  << " | findings=";

        for (std::size_t index = 0;
             index < incident.findingIds().size();
             ++index)
        {
            if (index > 0)
            {
                std::cout << ", ";
            }

            std::cout << incident.findingIds()[index];
        }

        std::cout << " | entities=";

        for (std::size_t index = 0;
             index < incident.entityIds().size();
             ++index)
        {
            if (index > 0)
            {
                std::cout << ", ";
            }

            std::cout << incident.entityIds()[index];
        }

        std::cout << '\n';
    }

    return 0;
}