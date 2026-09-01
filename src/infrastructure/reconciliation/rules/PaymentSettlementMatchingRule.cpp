#include "infrastructure/reconciliation/rules/PaymentSettlementMatchingRule.h"

#include <chrono>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fincon {

std::string PaymentSettlementMatchingRule::id() const
{
    return "PAYMENT_SETTLEMENT_MATCHING";
}

std::vector<ReconciliationFinding>
PaymentSettlementMatchingRule::evaluate(
    const FinancialDataset& dataset) const
{
    std::vector<ReconciliationFinding> findings;

    std::unordered_map<std::string, const Payment*> payments;

    std::unordered_map<std::string, int>
        paymentSettlementCount;

    std::unordered_map<
        std::string,
        std::vector<std::string>
    > paymentToSettlements;

    for (const Payment& payment : dataset.payments)
    {
        payments[payment.id] = &payment;
    }

    for (const Settlement& settlement : dataset.settlements)
    {
        for (const std::string& paymentId :
             settlement.paymentIds)
        {
            ++paymentSettlementCount[paymentId];

            paymentToSettlements[paymentId]
                .push_back(settlement.id);
        }
    }

    for (const Payment& payment : dataset.payments)
    {
        if (payment.status != PaymentStatus::Captured)
        {
            continue;
        }

        if (paymentSettlementCount[payment.id] != 0)
        {
            continue;
        }

        ReconciliationFinding finding;

        finding.type = FindingType::MissingRecord;
        finding.severity = FindingSeverity::High;

        finding.entityIds = {
            payment.id
        };

        finding.expectedValue =
            "settlement for payment";

        finding.observedValue =
            "settlement missing";

        finding.financialImpact =
            payment.amount;

        finding.ruleId = id();

        finding.description =
            "Captured payment does not have a corresponding settlement";

        finding.detectedAt =
            std::chrono::system_clock::now();

        findings.push_back(std::move(finding));
    }

    for (const auto& [paymentId, count] :
         paymentSettlementCount)
    {
        if (count <= 1)
        {
            continue;
        }

        ReconciliationFinding finding;

        finding.type = FindingType::DuplicateRecord;
        finding.severity = FindingSeverity::High;

        finding.entityIds.push_back(paymentId);

        for (const std::string& settlementId :
             paymentToSettlements[paymentId])
        {
            finding.entityIds.push_back(settlementId);
        }

        finding.expectedValue =
            "one settlement for payment";

        finding.observedValue =
            "multiple settlements for payment";

        const auto paymentIt =
            payments.find(paymentId);

        if (paymentIt != payments.end())
        {
            finding.financialImpact =
                paymentIt->second->amount;
        }

        finding.ruleId = id();

        finding.description =
            "Payment is referenced by multiple settlement records";

        finding.detectedAt =
            std::chrono::system_clock::now();

        findings.push_back(std::move(finding));
    }

    for (const Settlement& settlement :
         dataset.settlements)
    {
        for (const std::string& paymentId :
             settlement.paymentIds)
        {
            if (payments.find(paymentId) != payments.end())
            {
                continue;
            }

            ReconciliationFinding finding;

            finding.type = FindingType::MissingRecord;
            finding.severity = FindingSeverity::High;

            finding.entityIds = {
                settlement.id,
                paymentId
            };

            finding.expectedValue =
                "payment referenced by settlement";

            finding.observedValue =
                "payment missing";

            finding.financialImpact =
                settlement.netAmount;

            finding.ruleId = id();

            finding.description =
                "Settlement references a payment record that does not exist";

            finding.detectedAt =
                std::chrono::system_clock::now();

            findings.push_back(std::move(finding));
        }
    }

    for (const Settlement& settlement :
         dataset.settlements)
    {
        for (const std::string& paymentId :
             settlement.paymentIds)
        {
            const auto paymentIt =
                payments.find(paymentId);

            if (paymentIt == payments.end())
            {
                continue;
            }

            const Payment& payment =
                *paymentIt->second;

            if (payment.merchantId ==
                settlement.merchantId)
            {
                continue;
            }

            ReconciliationFinding finding;

            finding.type = FindingType::AmountMismatch;
            finding.severity = FindingSeverity::Critical;

            finding.entityIds = {
                payment.id,
                settlement.id
            };

            finding.expectedValue =
                "same merchant";

            finding.observedValue =
                "payment and settlement belong to different merchants";

            finding.financialImpact =
                payment.amount;

            finding.ruleId = id();

            finding.description =
                "Payment and settlement have different merchant IDs";

            finding.detectedAt =
                std::chrono::system_clock::now();

            findings.push_back(std::move(finding));
        }
    }

    for (const Settlement& settlement :
         dataset.settlements)
    {
        if (settlement.paymentIds.size() != 1)
        {
            continue;
        }

        const std::string& paymentId =
            settlement.paymentIds.front();

        const auto paymentIt =
            payments.find(paymentId);

        if (paymentIt == payments.end())
        {
            continue;
        }

        const Payment& payment =
            *paymentIt->second;

        if (settlement.grossAmount ==
            payment.amount)
        {
            continue;
        }

        const Money difference =
            settlement.grossAmount >= payment.amount
                ? settlement.grossAmount - payment.amount
                : payment.amount - settlement.grossAmount;

        ReconciliationFinding finding;

        finding.type = FindingType::AmountMismatch;
        finding.severity = FindingSeverity::High;

        finding.entityIds = {
            payment.id,
            settlement.id
        };

        finding.expectedValue =
            std::to_string(payment.amount);

        finding.observedValue =
            std::to_string(settlement.grossAmount);

        finding.financialImpact =
            difference;

        finding.ruleId = id();

        finding.description =
            "Settlement gross amount does not match the associated payment amount";

        finding.detectedAt =
            std::chrono::system_clock::now();

        findings.push_back(std::move(finding));
    }

    return findings;
}

}