# cmake_minimum_required(VERSION 3.20)

# project(
#     FinCon
#     VERSION 1.0.0
#     DESCRIPTION "Financial exception investigation and control system"
#     LANGUAGES CXX
# )

# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

# find_package(CURL REQUIRED)
# find_package(nlohmann_json REQUIRED)


# add_executable(fincon

#     src/main.cpp
#     src/infrastructure/generator/FinancialDataGenerator.cpp
#     src/infrastructure/exception/ExceptionScenario.cpp
#     src/infrastructure/exception/ExceptionInjector.cpp
#     src/infrastructure/reconciliation/ReconciliationEngine.cpp
#     src/infrastructure/reconciliation/rules/SettlementCalculationRule.cpp
#     src/infrastructure/reconciliation/rules/RefundReconciliationRule.cpp
#     src/infrastructure/reconciliation/rules/SettlementRefundRule.cpp
#     src/infrastructure/reconciliation/rules/SettlementFeeRule.cpp
#     src/infrastructure/reconciliation/rules/SettlementBankRule.cpp
#     src/infrastructure/reconciliation/rules/SettlementTimingRule.cpp
#     src/infrastructure/reconciliation/rules/MissingRecordRule.cpp
#     src/infrastructure/reconciliation/rules/DuplicateRecordRule.cpp
#     src/infrastructure/reconciliation/rules/PaymentSettlementMatchingRule.cpp
#     src/infrastructure/reconciliation/rules/SettlementAccountingRule.cpp
#     src/infrastructure/reconciliation/ReconciliationFindingCorrelator.cpp
#     src/domain/incident/IncidentBuilder.cpp
#     src/infrastructure/investigation/InMemoryFinancialDataRepository.cpp
#     src/application/investigation/InvestigationToolRegistry.cpp
#     src/application/investigation/DeterministicEvidenceProvider.cpp
#     src/application/investigation/DeterministicInvestigationPlanner.cpp
#     src/application/investigation/DefaultInvestigationService.cpp
#     src/infrastructure/investigation/tools/GetPaymentTool.cpp
#     src/infrastructure/investigation/tools/GetSettlementTool.cpp
#     src/infrastructure/investigation/tools/GetRefundsTool.cpp
#     src/infrastructure/investigation/tools/GetBankTransactionsTool.cpp
#     src/infrastructure/investigation/tools/GetAccountingEntriesTool.cpp
#     src/infrastructure/investigation/tools/GetRelatedTransactionsTool.cpp
#     src/infrastructure/investigation/tools/CalculateDifferenceTool.cpp
#     src/infrastructure/investigation/DeterministicHypothesisEngine.cpp
#     src/infrastructure/investigation/DeterministicImpactCalculator.cpp
#     src/infrastructure/investigation/DeterministicDecisionPolicy.cpp
#     src/infrastructure/investigation/DeterministicInvestigationEscalationPolicy.cpp
#     src/infrastructure/investigation/DeterministicInvestigationCompletenessEvaluator.cpp
#     src/infrastructure/investigation/DeterministicInvestigationToolRequestValidator.cpp
#     src/infrastructure/investigation/LibcurlHttpClient.cpp
#     src/infrastructure/configuration/DotEnvLoader.cpp
#     src/infrastructure/configuration/LLMConfiguration.cpp
#     src/infrastructure/investigation/MetaLlamaInvestigationAgent.cpp
#     src/infrastructure/investigation/MetaLlamaLLMProvider.cpp
#     src/infrastructure/investigation/JsonInvestigationResponseParser.cpp
#     src/application/investigation/InvestigationAgentOrchestrator.cpp
#     src/infrastructure/investigation/DeterministicInvestigationResponseValidator.cpp
#     src/application/evaluation/Phase2Evaluation.cpp
#     src/application/evaluation/Phase2EvaluationScenario.cpp
#     src/application/evaluation/Phase2EvaluationRunner.cpp


# )

# target_include_directories(fincon
#     PRIVATE
#         ${PROJECT_SOURCE_DIR}/include
# )

# target_link_libraries(fincon
#     PRIVATE
#         CURL::libcurl
#         nlohmann_json::nlohmann_json
# )

# if(MSVC)

#     target_compile_options(fincon PRIVATE
#         /W4
#         /permissive-
#     )

# else()

#     target_compile_options(fincon PRIVATE
#         -Wall
#         -Wextra
#         -Wpedantic
#         -Wconversion
#         -Wsign-conversion
#     )

# endif()


cmake_minimum_required(VERSION 3.20)

project(
    FinCon
    VERSION 1.0.0
    DESCRIPTION "Financial exception investigation and control system"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(fincon_core

    src/infrastructure/generator/FinancialDataGenerator.cpp

    src/infrastructure/exception/ExceptionScenario.cpp
    src/infrastructure/exception/ExceptionInjector.cpp

    src/infrastructure/reconciliation/ReconciliationEngine.cpp
    src/infrastructure/reconciliation/rules/SettlementCalculationRule.cpp
    src/infrastructure/reconciliation/rules/RefundReconciliationRule.cpp
    src/infrastructure/reconciliation/rules/SettlementRefundRule.cpp
    src/infrastructure/reconciliation/rules/SettlementFeeRule.cpp
    src/infrastructure/reconciliation/rules/SettlementBankRule.cpp
    src/infrastructure/reconciliation/rules/SettlementTimingRule.cpp
    src/infrastructure/reconciliation/rules/MissingRecordRule.cpp
    src/infrastructure/reconciliation/rules/DuplicateRecordRule.cpp
    src/infrastructure/reconciliation/rules/PaymentSettlementMatchingRule.cpp
    src/infrastructure/reconciliation/rules/SettlementAccountingRule.cpp
    src/infrastructure/reconciliation/ReconciliationFindingCorrelator.cpp

    src/domain/incident/IncidentBuilder.cpp

    src/infrastructure/investigation/InMemoryFinancialDataRepository.cpp

    src/application/investigation/InvestigationToolRegistry.cpp
    src/application/investigation/DeterministicEvidenceProvider.cpp
    src/application/investigation/DeterministicInvestigationPlanner.cpp
    src/application/investigation/DefaultInvestigationService.cpp

    src/infrastructure/investigation/tools/GetPaymentTool.cpp
    src/infrastructure/investigation/tools/GetSettlementTool.cpp
    src/infrastructure/investigation/tools/GetRefundsTool.cpp
    src/infrastructure/investigation/tools/GetBankTransactionsTool.cpp
    src/infrastructure/investigation/tools/GetAccountingEntriesTool.cpp
    src/infrastructure/investigation/tools/GetRelatedTransactionsTool.cpp
    src/infrastructure/investigation/tools/CalculateDifferenceTool.cpp

    src/infrastructure/investigation/DeterministicHypothesisEngine.cpp
    src/infrastructure/investigation/DeterministicImpactCalculator.cpp
    src/infrastructure/investigation/DeterministicDecisionPolicy.cpp
    src/infrastructure/investigation/DeterministicInvestigationEscalationPolicy.cpp
    src/infrastructure/investigation/DeterministicInvestigationCompletenessEvaluator.cpp
    src/infrastructure/investigation/DeterministicInvestigationToolRequestValidator.cpp
    src/infrastructure/investigation/DeterministicInvestigationResponseValidator.cpp

    src/infrastructure/investigation/LibcurlHttpClient.cpp

    src/infrastructure/configuration/DotEnvLoader.cpp
    src/infrastructure/configuration/LLMConfiguration.cpp

    src/infrastructure/investigation/MetaLlamaInvestigationAgent.cpp
    src/infrastructure/investigation/MetaLlamaLLMProvider.cpp
    src/infrastructure/investigation/JsonInvestigationResponseParser.cpp

    src/application/investigation/InvestigationAgentOrchestrator.cpp

    src/application/evaluation/Phase2Evaluation.cpp
    src/application/evaluation/Phase2EvaluationScenario.cpp
    src/application/evaluation/Phase2EvaluationRunner.cpp
    src/application/investigation/InvestigationAuditService.cpp
    src/infrastructure/investigation/InMemoryInvestigationAuditRepository.cpp
    src/infrastructure/investigation/DeterministicRecommendationPolicy.cpp
    src/application/controller/FinanceControllerFacade.cpp
    src/application/controller/FinanceControllerApi.cpp
    src/infrastructure/http/SimpleHttpServer.cpp
    src/application/state/FinanceControllerState.cpp
    src/application/ingestion/FinancialFieldMapper.cpp
)

target_include_directories(fincon_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(fincon_core
    PUBLIC
        CURL::libcurl
        nlohmann_json::nlohmann_json
)

if(MSVC)

    target_compile_options(fincon_core PRIVATE
        /W4
        /permissive-
    )

else()

    target_compile_options(fincon_core PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
    )

endif()

add_executable(fincon
    src/main.cpp
)

target_link_libraries(fincon
    PRIVATE
        fincon_core
)

add_executable(fincon_evaluation
    src/evaluation_main.cpp
)

target_link_libraries(fincon_evaluation
    PRIVATE
        fincon_core
)