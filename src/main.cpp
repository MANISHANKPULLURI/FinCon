#include "domain/incident/FindingCorrelation.h"
#include "domain/incident/IncidentBuilder.h"
#include "application/controller/FinanceControllerApi.h"
#include "application/controller/FinanceControllerFacade.h"
#include "application/evaluation/Phase2Evaluation.h"
#include "application/evaluation/Phase2EvaluationScenario.h"
#include "application/investigation/DefaultInvestigationService.h"
#include "application/investigation/DeterministicEvidenceProvider.h"
#include "application/investigation/DeterministicInvestigationPlanner.h"
#include "application/investigation/InvestigationAgentOrchestrator.h"
#include "application/investigation/InvestigationToolRegistry.h"
#include "application/ingestion/FinancialDataBatch.h"
#include "application/ingestion/MessageQueue.h"
#include "application/state/FinanceControllerState.h"
#include "application/investigation/LLMConfiguration.h"
#include "infrastructure/configuration/DotEnvLoader.h"
#include "infrastructure/exception/ExceptionInjector.h"
#include "infrastructure/generator/FinancialDataGenerator.h"
#include "infrastructure/investigation/DeterministicDecisionPolicy.h"
#include "infrastructure/investigation/DeterministicHypothesisEngine.h"
#include "infrastructure/investigation/DeterministicImpactCalculator.h"
#include "infrastructure/investigation/DeterministicInvestigationCompletenessEvaluator.h"
#include "infrastructure/investigation/DeterministicInvestigationEscalationPolicy.h"
#include "infrastructure/investigation/DeterministicInvestigationToolRequestValidator.h"
#include "infrastructure/investigation/DeterministicInvestigationResponseValidator.h"
#include "infrastructure/investigation/DeterministicRecommendationPolicy.h"
#include "infrastructure/investigation/InMemoryFinancialDataRepository.h"
#include "infrastructure/investigation/InMemoryInvestigationAuditRepository.h"
#include "application/investigation/InvestigationAuditService.h"
#include "infrastructure/investigation/JsonInvestigationResponseParser.h"
#include "infrastructure/investigation/LibcurlHttpClient.h"
#include "infrastructure/investigation/MetaLlamaInvestigationAgent.h"
#include "infrastructure/investigation/MetaLlamaLLMProvider.h"
#include "infrastructure/http/SimpleHttpServer.h"
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
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>
namespace
{
    std::atomic<bool> shutdownRequested{false};
    void requestShutdown(int) { shutdownRequested.store(true); }
    std::size_t workerCountFromEnv()
    {
        const char* v = std::getenv("FINCON_WORKERS");
        if (v == nullptr) return 1;
        try { auto n = std::stoul(v); return n == 0 ? 1 : n; } catch (...) { return 1; }
    }
}
int main()
{
    fincon::DotEnvLoader env("src/.env");
    std::string apiKey = env.get("MUSE_API_KEY");
    if (apiKey.empty()) { const char* v = std::getenv("MUSE_API_KEY"); if (v) apiKey = v; }
    std::string model = env.get("MUSE_MODEL");
    if (model.empty()) { const char* v = std::getenv("MUSE_MODEL"); if (v) model = v; }
    std::string baseUrl = env.get("MUSE_BASE_URL");
    if (baseUrl.empty()) { const char* v = std::getenv("MUSE_BASE_URL"); if (v) baseUrl = v; }
    fincon::LLMConfiguration llmConfiguration(apiKey, model, baseUrl);
    fincon::LibcurlHttpClient httpClient;
    fincon::MetaLlamaLLMProvider llmProvider(httpClient, llmConfiguration.apiKey(), llmConfiguration.model(), llmConfiguration.baseUrl());
    fincon::JsonInvestigationResponseParser responseParser;
    fincon::MetaLlamaInvestigationAgent investigationAgent(llmProvider, responseParser);
    fincon::InMemoryFinancialDataRepository repository({}, {}, {}, {}, {}, {});
    fincon::InMemoryInvestigationAuditRepository auditRepository;
    fincon::InvestigationAuditService auditService(auditRepository);
    fincon::DeterministicEvidenceProvider evidenceProvider(repository);
    fincon::DeterministicHypothesisEngine hypothesisEngine;
    fincon::DeterministicImpactCalculator impactCalculator;
    fincon::DeterministicDecisionPolicy decisionPolicy;
    fincon::InvestigationToolRegistry toolRegistry;
    toolRegistry.registerTool("get_payment", std::make_unique<fincon::GetPaymentTool>(repository));
    toolRegistry.registerTool("get_settlement", std::make_unique<fincon::GetSettlementTool>(repository));
    toolRegistry.registerTool("get_refunds", std::make_unique<fincon::GetRefundsTool>(repository));
    toolRegistry.registerTool("get_bank_transactions", std::make_unique<fincon::GetBankTransactionsTool>(repository));
    toolRegistry.registerTool("get_accounting_entries", std::make_unique<fincon::GetAccountingEntriesTool>(repository));
    toolRegistry.registerTool("get_related_transactions", std::make_unique<fincon::GetRelatedTransactionsTool>(repository));
    toolRegistry.registerTool("calculate_difference", std::make_unique<fincon::CalculateDifferenceTool>(repository));
    fincon::DeterministicInvestigationToolRequestValidator toolRequestValidator;
    fincon::DeterministicInvestigationResponseValidator responseValidator;
    fincon::InvestigationAgentOrchestrator agentOrchestrator(investigationAgent, toolRegistry, toolRequestValidator, responseValidator);
    fincon::DeterministicInvestigationPlanner planner;
    fincon::DeterministicInvestigationCompletenessEvaluator completenessEvaluator;
    fincon::DeterministicInvestigationEscalationPolicy escalationPolicy;
    fincon::DeterministicRecommendationPolicy recommendationPolicy;
    fincon::DefaultInvestigationService investigationService(planner, evidenceProvider, hypothesisEngine, impactCalculator, decisionPolicy, toolRegistry, completenessEvaluator, escalationPolicy, agentOrchestrator, auditService, recommendationPolicy);
    fincon::ReconciliationEngine reconciliationEngine;
    reconciliationEngine.addRule(std::make_unique<fincon::SettlementCalculationRule>());
    reconciliationEngine.addRule(std::make_unique<fincon::SettlementRefundRule>());
    reconciliationEngine.addRule(std::make_unique<fincon::SettlementFeeRule>());
    reconciliationEngine.addRule(std::make_unique<fincon::SettlementBankRule>());
    reconciliationEngine.addRule(std::make_unique<fincon::SettlementTimingRule>());
    reconciliationEngine.addRule(std::make_unique<fincon::MissingRecordRule>());
    reconciliationEngine.addRule(std::make_unique<fincon::DuplicateRecordRule>());
    reconciliationEngine.addRule(std::make_unique<fincon::PaymentSettlementMatchingRule>());
    reconciliationEngine.addRule(std::make_unique<fincon::SettlementAccountingRule>());
    fincon::ReconciliationFindingCorrelator correlator;
    fincon::IncidentBuilder incidentBuilder;
    fincon::FinanceControllerState state;
    fincon::FinanceControllerFacade facade(investigationService, auditRepository, state);
    fincon::MessageQueue<fincon::FinancialDataBatch> batchQueue(32);
    fincon::FinanceControllerApi api(facade, batchQueue, state);
    fincon::SimpleHttpServer server;
    fincon::Phase2Evaluation evaluation;
    api.registerRoutes(server);
    server.start(8080);
    std::signal(SIGINT, requestShutdown);
    std::signal(SIGTERM, requestShutdown);
    std::cout << "FinCon HTTP server running on port 8080\nHealth: http://localhost:8080/health\n";
    const std::size_t workers = workerCountFromEnv();
    state.setActiveWorkers(workers);
    std::vector<std::thread> pool;
    pool.reserve(workers);
    auto processBatch = [&](fincon::FinancialDataBatch batch)
    {
        std::string bid = batch.batchId;
        state.emitEvent("batch_processing", bid);
        repository.appendBatch(batch.data);
        state.emitEvent("reconciliation_completed", bid);
        const auto batchFindings = reconciliationEngine.reconcile(batch.data);
        if (!batchFindings.empty()) state.emitEvent("exception_detected", bid + ":" + std::to_string(batchFindings.size()));
        const auto batchCorrelations = correlator.correlate(batchFindings);
        auto batchIncidents = incidentBuilder.build(batchCorrelations);
        for (auto& bi : batchIncidents) bi.setId(bid + "-" + bi.id());
        for (auto& bi : batchIncidents) state.emitEvent("incident_created", bi.id());
        state.addIncidents(batchIncidents);
        for (const auto& bi : batchIncidents)
        {
            state.emitEvent("investigation_started", bi.id());
            for (auto &ev : {"tool_started","evidence_collected","hypothesis_generated","impact_calculated","decision_made","recommendation_created"}) state.emitEvent(ev, bi.id());
            auto inv = facade.investigate(bi);
            for (auto &tc : inv.toolCalls()) { state.emitEvent("tool_started", tc.toolName()); state.emitEvent("tool_completed", tc.toolName()); }
            if (!inv.evidence().empty()) state.emitEvent("evidence_collected", bi.id());
            if (!inv.hypotheses().empty()) state.emitEvent("hypothesis_generated", bi.id());
            state.emitEvent("impact_calculated", bi.id() + ":" + std::to_string(inv.confirmedImpact()));
            state.emitEvent("decision_made", bi.id() + ":" + std::to_string(static_cast<int>(inv.outcome())));
            if (inv.recommendation()) state.emitEvent("recommendation_created", bi.id());
            state.emitEvent("investigation_completed", bi.id());
        }
        state.markBatchProcessed(batch.data.payments.size());
        state.emitEvent("processing_completed", bid);
    };
    std::thread initializer([&]()
    {
        state.setProcessingStatus("running");
        state.emitEvent("batch_received", "INIT");
        try
        {
            constexpr std::uint64_t seed = 42;
            constexpr std::uint32_t exceptionRatePercent = 30;
            fincon::FinancialDataGenerator generator(seed);
            fincon::FinancialDataset dataset = generator.generate(2, 10);
            fincon::ExceptionInjector injector(seed, fincon::ExceptionInjectionMode::Coverage);
            const std::vector<fincon::InjectedException> exceptions = injector.inject(dataset, exceptionRatePercent);
            repository.appendBatch(dataset);
            state.emitEvent("reconciliation_completed", "INIT");
            const std::vector<fincon::ReconciliationFinding> findings = reconciliationEngine.reconcile(dataset);
            if (!findings.empty()) state.emitEvent("exception_detected", std::to_string(findings.size()));
            const std::vector<fincon::FindingCorrelation> correlations = correlator.correlate(findings);
            const std::vector<fincon::Incident> incidents = incidentBuilder.build(correlations);
            for (auto &inc: incidents) state.emitEvent("incident_created", inc.id());
            state.setIncidents(incidents);
            state.setTotalRecords(dataset.payments.size() + dataset.refunds.size() + dataset.fees.size() + dataset.settlements.size() + dataset.bankTransactions.size() + dataset.accountingEntries.size());
            state.setInitialProcessedRecords(dataset.payments.size());
            std::cout << "Initial dataset: payments=" << dataset.payments.size() << " findings=" << findings.size() << " incidents=" << incidents.size() << "\n";
            for (const auto& inc : incidents)
            {
                if (shutdownRequested.load()) break;
                state.emitEvent("investigation_started", inc.id());
                fincon::Investigation pending("INV-" + inc.id());
                pending.setIncidentId(inc.id());
                pending.setStatus(fincon::InvestigationStatus::Pending);
                state.setInvestigation(std::move(pending));
            }
            std::vector<fincon::Investigation> completed;
            completed.reserve(incidents.size());
            for (const auto& inc : incidents)
            {
                if (shutdownRequested.load()) break;
                auto inv = facade.investigate(inc);
                state.emitEvent("investigation_completed", inc.id());
                state.emitEvent("decision_made", inc.id());
                completed.push_back(std::move(inv));
            }
            fincon::Phase2EvaluationScenarioSet scenarioSet;
            state.setEvaluation(evaluation.evaluate(dataset.payments.size() + dataset.refunds.size() + dataset.fees.size() + dataset.settlements.size() + dataset.bankTransactions.size() + dataset.accountingEntries.size(), exceptions.size(), incidents, completed, scenarioSet.generate(exceptions.size())));
            state.emitEvent("processing_completed", "INIT");
        }
        catch (const std::exception& ex) { state.setProcessingStatus("failed", ex.what()); }
    });
    for (std::size_t i = 0; i < workers; ++i)
    {
        pool.emplace_back([&]()
        {
            while (auto batch = batchQueue.pop())
            {
                if (shutdownRequested.load()) break;
                try { processBatch(std::move(*batch)); } catch (...) {}
            }
        });
    }
    while (!shutdownRequested.load()) std::this_thread::sleep_for(std::chrono::seconds(1));
    batchQueue.shutdown();
    if (initializer.joinable()) initializer.join();
    for (auto& t : pool) if (t.joinable()) t.join();
    server.stop();
}
