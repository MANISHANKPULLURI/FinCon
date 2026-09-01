#include "infrastructure/reconciliation/ReconciliationEngine.h"

#include <stdexcept>
#include <string>
#include <utility>

namespace fincon {

ReconciliationEngine::ReconciliationEngine() = default;

void ReconciliationEngine::addRule(
    std::unique_ptr<ReconciliationRule> rule) {

    if (!rule) {
        throw std::invalid_argument(
            "Reconciliation rule cannot be null"
        );
    }

    rules_.push_back(std::move(rule));
}

std::vector<ReconciliationFinding>
ReconciliationEngine::reconcile(
    const FinancialDataset& dataset) const {

    std::vector<ReconciliationFinding> findings;

    for (const auto& rule : rules_) {

        if (!rule) {
            continue;
        }

        std::vector<ReconciliationFinding> ruleFindings =
            rule->evaluate(dataset);

        findings.insert(
            findings.end(),
            ruleFindings.begin(),
            ruleFindings.end()
        );
    }

    for (std::size_t index = 0;
         index < findings.size();
         ++index) {

        findings[index].id =
            "RF-" + std::to_string(index + 1);
    }

    return findings;
}

}