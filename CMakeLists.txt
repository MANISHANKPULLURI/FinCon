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

add_executable(fincon
    src/main.cpp
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
    src/infrastructure/reconciliation/ReconciliationFindingCorrelator.cpp
    src/infrastructure/reconciliation/rules/SettlementAccountingRule.cpp
    src/domain/incident/IncidentBuilder.cpp
    src/infrastructure/investigation/InMemoryFinancialDataRepository.cpp
    src/application/investigation/InvestigationToolRegistry.cpp
    src/application/investigation/DeterministicEvidenceProvider.cpp
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
    src/application/investigation/DeterministicInvestigationPlanner.cpp
    src/application/investigation/DefaultInvestigationService.cpp
)

target_include_directories(fincon
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(fincon PRIVATE
        /W4
        /permissive-
    )
else()
    target_compile_options(fincon PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
    )
endif()