#include "application/evaluation/Phase2Evaluation.h"
#include "application/evaluation/Phase2EvaluationRunner.h"

#include "application/investigation/DefaultInvestigationService.h"
#include "application/investigation/InvestigationAgentOrchestrator.h"
#include "application/investigation/DeterministicInvestigationPlanner.h"
#include "application/investigation/InvestigationToolRegistry.h"
#include "application/investigation/LLMConfiguration.h"
#include "application/investigation/RecommendationPolicy.h"

#include "domain/incident/IncidentBuilder.h"
#include "domain/incident/FindingCorrelation.h"

#include "infrastructure/configuration/DotEnvLoader.h"
#include "infrastructure/exception/ExceptionInjector.h"
#include "infrastructure/generator/FinancialDataGenerator.h"

#include "infrastructure/investigation/DeterministicDecisionPolicy.h"
#include "application/investigation/DeterministicEvidenceProvider.h"
#include "infrastructure/investigation/DeterministicHypothesisEngine.h"
#include "infrastructure/investigation/DeterministicImpactCalculator.h"
#include "infrastructure/investigation/DeterministicInvestigationCompletenessEvaluator.h"
#include "infrastructure/investigation/DeterministicInvestigationEscalationPolicy.h"
#include "infrastructure/investigation/DeterministicInvestigationResponseValidator.h"
#include "infrastructure/investigation/DeterministicInvestigationToolRequestValidator.h"
#include "infrastructure/investigation/DeterministicRecommendationPolicy.h"
#include "infrastructure/investigation/InMemoryFinancialDataRepository.h"
#include "infrastructure/investigation/InMemoryInvestigationAuditRepository.h"
#include "application/investigation/InvestigationAuditService.h"

#include "infrastructure/investigation/JsonInvestigationResponseParser.h"
#include "infrastructure/investigation/LibcurlHttpClient.h"
#include "infrastructure/investigation/MetaLlamaInvestigationAgent.h"
#include "infrastructure/investigation/MetaLlamaLLMProvider.h"

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

#include "infrastructure/investigation/tools/CalculateDifferenceTool.h"
#include "infrastructure/investigation/tools/GetAccountingEntriesTool.h"
#include "infrastructure/investigation/tools/GetBankTransactionsTool.h"
#include "infrastructure/investigation/tools/GetPaymentTool.h"
#include "infrastructure/investigation/tools/GetRefundsTool.h"
#include "infrastructure/investigation/tools/GetRelatedTransactionsTool.h"
#include "infrastructure/investigation/tools/GetSettlementTool.h"

#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

int main()
{
    constexpr std::uint64_t seed = 42;
    constexpr std::uint32_t exceptionRatePercent = 30;

    constexpr std::size_t merchantCount = 100;
    constexpr std::size_t ordersPerMerchant = 10;

    fincon::DotEnvLoader env("src/.env");

    fincon::LLMConfiguration llmConfiguration(
        env.get("MUSE_API_KEY"),
        env.get("MUSE_MODEL")
    );

    fincon::LibcurlHttpClient httpClient;

    fincon::MetaLlamaLLMProvider llmProvider(
        httpClient,
        llmConfiguration.apiKey(),
        llmConfiguration.model()
    );

    fincon::JsonInvestigationResponseParser responseParser;

    fincon::MetaLlamaInvestigationAgent investigationAgent(
        llmProvider,
        responseParser
    );

    fincon::FinancialDataGenerator generator(seed);

    fincon::FinancialDataset dataset =
        generator.generate(
            merchantCount,
            ordersPerMerchant
        );

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

    fincon::InMemoryFinancialDataRepository repository(
        dataset.payments,
        dataset.settlements,
        dataset.refunds,
        dataset.bankTransactions,
        dataset.accountingEntries,
        dataset.fees
    );

    fincon::InMemoryInvestigationAuditRepository auditRepository;
    fincon::InvestigationAuditService auditService(auditRepository);

    fincon::DeterministicEvidenceProvider evidenceProvider(
        repository
    );

    fincon::DeterministicHypothesisEngine hypothesisEngine;

    fincon::DeterministicImpactCalculator impactCalculator;

    fincon::DeterministicDecisionPolicy decisionPolicy;

    fincon::InvestigationToolRegistry toolRegistry;

    toolRegistry.registerTool(
        "get_payment",
        std::make_unique<fincon::GetPaymentTool>(repository)
    );

    toolRegistry.registerTool(
        "get_settlement",
        std::make_unique<fincon::GetSettlementTool>(repository)
    );

    toolRegistry.registerTool(
        "get_refunds",
        std::make_unique<fincon::GetRefundsTool>(repository)
    );

    toolRegistry.registerTool(
        "get_bank_transactions",
        std::make_unique<fincon::GetBankTransactionsTool>(repository)
    );

    toolRegistry.registerTool(
        "get_accounting_entries",
        std::make_unique<fincon::GetAccountingEntriesTool>(repository)
    );

    toolRegistry.registerTool(
        "get_related_transactions",
        std::make_unique<fincon::GetRelatedTransactionsTool>(repository)
    );

    toolRegistry.registerTool(
        "calculate_difference",
        std::make_unique<fincon::CalculateDifferenceTool>(repository)
    );

    fincon::DeterministicInvestigationToolRequestValidator
        toolRequestValidator;

    fincon::DeterministicInvestigationResponseValidator
        responseValidator;

    fincon::InvestigationAgentOrchestrator agentOrchestrator(
        investigationAgent,
        toolRegistry,
        toolRequestValidator,
        responseValidator
    );

    fincon::DeterministicInvestigationPlanner planner;

    fincon::DeterministicInvestigationCompletenessEvaluator
        completenessEvaluator;

    fincon::DeterministicInvestigationEscalationPolicy
        escalationPolicy;

    fincon::DeterministicRecommendationPolicy recommendationPolicy;

    fincon::DefaultInvestigationService investigationService(
        planner,
        evidenceProvider,
        hypothesisEngine,
        impactCalculator,
        decisionPolicy,
        toolRegistry,
        completenessEvaluator,
        escalationPolicy,
        agentOrchestrator,
        auditService,
        recommendationPolicy
    );

    fincon::Phase2Evaluation evaluation;

    fincon::Phase2EvaluationRunner runner(
        investigationService,
        evaluation
    );

    const fincon::Phase2EvaluationResult result =
        runner.run(
            dataset,
            exceptions,
            incidents
        );

    std::cout << "FinCon Phase 2 Evaluation\n\n";

    std::cout << "Merchants: "
              << dataset.merchants.size()
              << '\n';

    std::cout << "Orders: "
              << dataset.orders.size()
              << '\n';

    std::cout << "Payments: "
              << dataset.payments.size()
              << '\n';

    std::cout << "Settlements: "
              << dataset.settlements.size()
              << '\n';

    std::cout << "Injected exceptions: "
              << result.injectedExceptions
              << '\n';

    std::cout << "Reconciliation findings: "
              << findings.size()
              << '\n';

    std::cout << "Finding correlations: "
              << correlations.size()
              << '\n';

    std::cout << "Incidents: "
              << result.incidents
              << '\n';

    std::cout << "Investigations: "
              << result.investigations
              << '\n';

    std::cout << "Deterministic resolutions: "
              << result.deterministicResolutions
              << '\n';

    std::cout << "LLM resolutions: "
              << result.llmResolutions
              << '\n';

    std::cout << "AUTO_RESOLVE: "
              << result.autoResolved
              << '\n';

    std::cout << "HUMAN_REVIEW: "
              << result.humanReview
              << '\n';

    std::cout << "REQUEST_MORE_EVIDENCE: "
              << result.requestMoreEvidence
              << '\n';

    std::cout << "UNRESOLVED: "
              << result.unresolved
              << '\n';

    std::cout << "Correct incident types: "
              << result.correctIncidentTypes
              << '/'
              << result.investigations
              << '\n';

    std::cout << "Correct outcomes: "
              << result.correctOutcomes
              << '/'
              << result.investigations
              << '\n';

    std::cout << "Correct impacts: "
              << result.correctImpacts
              << '/'
              << result.investigations
              << '\n';

    std::cout << "Correct LLM escalations: "
              << result.correctLLMEscalations
              << '/'
              << result.investigations
              << '\n';

    std::cout << "Correctly evaluated: "
              << result.correctlyEvaluated
              << '/'
              << result.investigations
              << '\n';

    std::cout << "Evaluation: "
              << (result.passed ? "PASSED" : "FAILED")
              << '\n';

    return result.passed ? 0 : 1;
}