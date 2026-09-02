#include "infrastructure/investigation/DeterministicHypothesisEngine.h"

#include <algorithm>
#include <string>
#include <vector>

namespace fincon
{
    namespace
    {
        bool hasEvidenceType(
            const std::vector<InvestigationEvidence>& evidence,
            EvidenceType type)
        {
            return std::any_of(
                evidence.begin(),
                evidence.end(),
                [type](const InvestigationEvidence& item)
                {
                    return item.type() == type;
                }
            );
        }

        int countEvidenceType(
            const std::vector<InvestigationEvidence>& evidence,
            EvidenceType type)
        {
            return static_cast<int>(
                std::count_if(
                    evidence.begin(),
                    evidence.end(),
                    [type](const InvestigationEvidence& item)
                    {
                        return item.type() == type;
                    }
                )
            );
        }

        InvestigationHypothesis createHypothesis(
            const std::string& id,
            const std::string& description,
            HypothesisStatus status,
            int confidenceScore,
            Money estimatedImpact)
        {
            InvestigationHypothesis hypothesis(id);

            hypothesis.setDescription(description);
            hypothesis.setStatus(status);
            hypothesis.setConfidenceScore(confidenceScore);
            hypothesis.setEstimatedImpact(estimatedImpact);

            return hypothesis;
        }

        void addEvidenceByType(
            InvestigationHypothesis& hypothesis,
            const std::vector<InvestigationEvidence>& evidence,
            EvidenceType type)
        {
            for (const InvestigationEvidence& item : evidence)
            {
                if (item.type() == type)
                {
                    hypothesis.addEvidence(item.id());
                }
            }
        }

        void addEvidenceByTypes(
            InvestigationHypothesis& hypothesis,
            const std::vector<InvestigationEvidence>& evidence,
            EvidenceType first,
            EvidenceType second)
        {
            addEvidenceByType(hypothesis, evidence, first);
            addEvidenceByType(hypothesis, evidence, second);
        }
    }

    std::vector<InvestigationHypothesis>
    DeterministicHypothesisEngine::generate(
        const Incident& incident,
        const std::vector<InvestigationEvidence>& evidence) const
    {
        std::vector<InvestigationHypothesis> hypotheses;

        if (evidence.empty())
        {
            return hypotheses;
        }

        const Money incidentImpact = incident.financialImpact();

        if (incident.type() == IncidentType::DuplicateRecord &&
            countEvidenceType(evidence, EvidenceType::Settlement) > 1)
        {
            InvestigationHypothesis hypothesis =
                createHypothesis(
                    "HYP-DUPLICATE-1",
                    "Multiple settlement records are associated with the same payment",
                    HypothesisStatus::Supported,
                    95,
                    incidentImpact
                );

            addEvidenceByType(
                hypothesis,
                evidence,
                EvidenceType::Settlement
            );

            hypotheses.push_back(std::move(hypothesis));
        }

        if (incident.type() == IncidentType::RefundIssue &&
            hasEvidenceType(evidence, EvidenceType::Payment) &&
            hasEvidenceType(evidence, EvidenceType::Refund))
        {
            InvestigationHypothesis hypothesis =
                createHypothesis(
                    "HYP-REFUND-1",
                    "Refund activity does not reconcile with the investigated settlement",
                    HypothesisStatus::Supported,
                    90,
                    incidentImpact
                );

            addEvidenceByTypes(
                hypothesis,
                evidence,
                EvidenceType::Payment,
                EvidenceType::Refund
            );

            hypotheses.push_back(std::move(hypothesis));
        }

        if (incident.type() == IncidentType::FeeIssue &&
            hasEvidenceType(evidence, EvidenceType::Payment) &&
            hasEvidenceType(evidence, EvidenceType::RelatedTransaction))
        {
            InvestigationHypothesis hypothesis =
                createHypothesis(
                    "HYP-FEE-1",
                    "Fee activity does not reconcile with the investigated settlement",
                    HypothesisStatus::Supported,
                    90,
                    incidentImpact
                );

            addEvidenceByTypes(
                hypothesis,
                evidence,
                EvidenceType::Payment,
                EvidenceType::RelatedTransaction
            );

            hypotheses.push_back(std::move(hypothesis));
        }

        if (incident.type() == IncidentType::TimingIssue &&
            hasEvidenceType(evidence, EvidenceType::Settlement) &&
            hasEvidenceType(evidence, EvidenceType::BankTransaction))
        {
            InvestigationHypothesis hypothesis =
                createHypothesis(
                    "HYP-TIMING-1",
                    "Bank transaction timing differs from the expected settlement timing",
                    HypothesisStatus::Supported,
                    95,
                    incidentImpact
                );

            addEvidenceByTypes(
                hypothesis,
                evidence,
                EvidenceType::Settlement,
                EvidenceType::BankTransaction
            );

            hypotheses.push_back(std::move(hypothesis));
        }

        if (incident.type() == IncidentType::SettlementIssue &&
            hasEvidenceType(evidence, EvidenceType::Settlement))
        {
            InvestigationHypothesis hypothesis =
                createHypothesis(
                    "HYP-SETTLEMENT-1",
                    "Settlement amount does not reconcile with the associated financial records",
                    HypothesisStatus::Supported,
                    90,
                    incidentImpact
                );

            addEvidenceByType(
                hypothesis,
                evidence,
                EvidenceType::Settlement
            );

            addEvidenceByType(
                hypothesis,
                evidence,
                EvidenceType::BankTransaction
            );

            addEvidenceByType(
                hypothesis,
                evidence,
                EvidenceType::AccountingEntry
            );

            hypotheses.push_back(std::move(hypothesis));
        }

        if (incident.type() == IncidentType::BankIssue &&
            hasEvidenceType(evidence, EvidenceType::Settlement) &&
            hasEvidenceType(evidence, EvidenceType::BankTransaction))
        {
            InvestigationHypothesis hypothesis =
                createHypothesis(
                    "HYP-BANK-1",
                    "Bank transaction does not reconcile with the investigated settlement",
                    HypothesisStatus::Supported,
                    90,
                    incidentImpact
                );

            addEvidenceByTypes(
                hypothesis,
                evidence,
                EvidenceType::Settlement,
                EvidenceType::BankTransaction
            );

            hypotheses.push_back(std::move(hypothesis));
        }

        if (incident.type() == IncidentType::AccountingIssue &&
            hasEvidenceType(evidence, EvidenceType::Settlement) &&
            hasEvidenceType(evidence, EvidenceType::AccountingEntry))
        {
            InvestigationHypothesis hypothesis =
                createHypothesis(
                    "HYP-ACCOUNTING-1",
                    "Accounting entry does not reconcile with the investigated settlement",
                    HypothesisStatus::Supported,
                    90,
                    incidentImpact
                );

            addEvidenceByTypes(
                hypothesis,
                evidence,
                EvidenceType::Settlement,
                EvidenceType::AccountingEntry
            );

            hypotheses.push_back(std::move(hypothesis));
        }

        return hypotheses;
    }
}