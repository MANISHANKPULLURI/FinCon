#include "infrastructure/investigation/DeterministicInvestigationResponseValidator.h"

#include <string>
#include <unordered_set>

namespace fincon
{
    bool DeterministicInvestigationResponseValidator::validate(
        const InvestigationResponse& response,
        const InvestigationRequest& request,
        std::string& error) const
    {
        const InvestigationDecision& decision = response.decision();

        if (decision.financialImpact() < 0)
        {
            error = "Investigation financial impact cannot be negative";
            return false;
        }

        std::unordered_set<std::string> evidenceIds;

        for (const InvestigationEvidence& evidence : request.evidence())
            evidenceIds.insert(evidence.id());

        for (const std::string& evidenceId : decision.evidenceIds())
        {
            if (!evidenceIds.contains(evidenceId))
            {
                error =
                    "Investigation decision references unknown evidence: " +
                    evidenceId;

                return false;
            }
        }

        for (const InvestigationHypothesis& hypothesis :
             response.hypotheses())
        {
            if (hypothesis.id().empty())
            {
                error = "Investigation hypothesis ID is empty";
                return false;
            }

            if (hypothesis.confidenceScore() < 0 ||
                hypothesis.confidenceScore() > 100)
            {
                error =
                    "Investigation hypothesis confidence score is invalid";

                return false;
            }

            if (hypothesis.estimatedImpact() < 0)
            {
                error =
                    "Investigation hypothesis impact cannot be negative";

                return false;
            }

            for (const std::string& evidenceId :
                 hypothesis.evidenceIds())
            {
                if (!evidenceIds.contains(evidenceId))
                {
                    error =
                        "Investigation hypothesis references unknown evidence: " +
                        evidenceId;

                    return false;
                }
            }
        }

        if (decision.type() == DecisionType::AutoResolve)
        {
            if (decision.confidence() != DecisionConfidence::High)
            {
                error =
                    "AUTO_RESOLVE requires high confidence";

                return false;
            }

            if (decision.evidenceIds().empty())
            {
                error =
                    "AUTO_RESOLVE requires evidence";

                return false;
            }

            if (response.requiresMoreEvidence())
            {
                error =
                    "AUTO_RESOLVE cannot require more evidence";

                return false;
            }
        }

        return true;
    }
}