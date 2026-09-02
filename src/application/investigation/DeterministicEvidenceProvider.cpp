#include "application/investigation/DeterministicEvidenceProvider.h"

#include <string>
#include <vector>

namespace fincon
{
    namespace
    {
        InvestigationEvidence createEvidence(
            EvidenceType type,
            EvidenceStrength strength,
            const std::string& sourceId,
            const std::string& description,
            Money financialImpact)
        {
            InvestigationEvidence evidence(
                "EV-" + sourceId
            );

            evidence.setType(type);
            evidence.setStrength(strength);
            evidence.setSourceId(sourceId);
            evidence.setDescription(description);
            evidence.setFinancialImpact(financialImpact);

            return evidence;
        }
    }

    DeterministicEvidenceProvider::DeterministicEvidenceProvider(
        const FinancialDataRepository& repository)
        : repository_(repository)
    {
    }

    std::vector<InvestigationEvidence>
    DeterministicEvidenceProvider::collect(
        const std::vector<std::string>& entityIds) const
    {
        std::vector<InvestigationEvidence> evidence;

        for (const std::string& entityId : entityIds)
        {
            if (entityId.empty())
            {
                continue;
            }

            if (const Payment* payment =
                    repository_.getPayment(entityId))
            {
                evidence.push_back(
                    createEvidence(
                        EvidenceType::Payment,
                        EvidenceStrength::Strong,
                        payment->id,
                        "Payment record found",
                        payment->amount
                    )
                );

                continue;
            }

            if (const Settlement* settlement =
                    repository_.getSettlement(entityId))
            {
                evidence.push_back(
                    createEvidence(
                        EvidenceType::Settlement,
                        EvidenceStrength::Strong,
                        settlement->id,
                        "Settlement record found",
                        settlement->netAmount
                    )
                );

                continue;
            }

            if (const Refund* refund =
                    repository_.getRefund(entityId))
            {
                evidence.push_back(
                    createEvidence(
                        EvidenceType::Refund,
                        EvidenceStrength::Strong,
                        refund->id,
                        "Refund record found",
                        refund->amount
                    )
                );

                continue;
            }

            if (const BankTransaction* transaction =
                    repository_.getBankTransaction(entityId))
            {
                evidence.push_back(
                    createEvidence(
                        EvidenceType::BankTransaction,
                        EvidenceStrength::Strong,
                        transaction->id,
                        "Bank transaction found",
                        transaction->amount
                    )
                );

                continue;
            }

            if (const AccountingEntry* entry =
                    repository_.getAccountingEntry(entityId))
            {
                evidence.push_back(
                    createEvidence(
                        EvidenceType::AccountingEntry,
                        EvidenceStrength::Strong,
                        entry->id,
                        "Accounting entry found",
                        entry->amount
                    )
                );

                continue;
            }

            if (const Fee* fee =
                    repository_.getFee(entityId))
            {
                evidence.push_back(
                    createEvidence(
                        EvidenceType::RelatedTransaction,
                        EvidenceStrength::Strong,
                        fee->id,
                        "Fee record found",
                        fee->amount
                    )
                );
            }
        }

        return evidence;
    }
}