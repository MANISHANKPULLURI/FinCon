#include "infrastructure/investigation/JsonInvestigationResponseParser.h"

#include <nlohmann/json.hpp>

#include <stdexcept>
#include <string>
#include <utility>

namespace fincon
{
    namespace
    {
        std::string extractJson(const std::string& response)
        {
            const std::size_t objectStart =
                response.find('{');

            const std::size_t objectEnd =
                response.rfind('}');

            if (objectStart == std::string::npos ||
                objectEnd == std::string::npos ||
                objectStart > objectEnd)
            {
                throw std::invalid_argument(
                    "LLM response does not contain a JSON object"
                );
            }

            return response.substr(
                objectStart,
                objectEnd - objectStart + 1
            );
        }

        DecisionType parseDecisionType(
            const std::string& value)
        {
            if (value == "AUTO_RESOLVE")
                return DecisionType::AutoResolve;

            if (value == "HUMAN_REVIEW")
                return DecisionType::HumanReview;

            if (value == "REQUEST_MORE_EVIDENCE")
                return DecisionType::RequestMoreEvidence;

            if (value == "UNRESOLVED")
                return DecisionType::Unresolved;

            throw std::invalid_argument(
                "Invalid investigation decision: " + value
            );
        }

        DecisionConfidence parseConfidence(
            const std::string& value)
        {
            if (value == "LOW")
                return DecisionConfidence::Low;

            if (value == "MEDIUM")
                return DecisionConfidence::Medium;

            if (value == "HIGH")
                return DecisionConfidence::High;

            throw std::invalid_argument(
                "Invalid investigation confidence: " + value
            );
        }

        HypothesisStatus parseHypothesisStatus(
            const nlohmann::json& value)
        {
            if (value.is_number_integer())
            {
                switch (value.get<int>())
                {
                case 0:
                    return HypothesisStatus::Proposed;

                case 1:
                    return HypothesisStatus::Testing;

                case 2:
                    return HypothesisStatus::Supported;

                case 3:
                    return HypothesisStatus::Rejected;

                case 4:
                    return HypothesisStatus::Inconclusive;

                default:
                    return HypothesisStatus::Inconclusive;
                }
            }

            if (value.is_string())
            {
                const std::string status =
                    value.get<std::string>();

                if (status == "PROPOSED")
                    return HypothesisStatus::Proposed;

                if (status == "TESTING")
                    return HypothesisStatus::Testing;

                if (status == "SUPPORTED")
                    return HypothesisStatus::Supported;

                if (status == "REJECTED")
                    return HypothesisStatus::Rejected;

                if (status == "INCONCLUSIVE")
                    return HypothesisStatus::Inconclusive;

                if (status == "PENDING_VALIDATION")
                    return HypothesisStatus::Testing;

                if (status == "UNCONFIRMED")
                    return HypothesisStatus::Inconclusive;

                if (status == "UNKNOWN")
                    return HypothesisStatus::Inconclusive;
            }

            return HypothesisStatus::Inconclusive;
        }
    }

    InvestigationResponse
    JsonInvestigationResponseParser::parse(
        const std::string& response) const
    {
        if (response.empty())
            throw std::invalid_argument(
                "LLM response is empty"
            );

        const std::string jsonText =
            extractJson(response);

        const nlohmann::json json =
            nlohmann::json::parse(jsonText);

        if (!json.is_object())
            throw std::invalid_argument(
                "LLM response must be a JSON object"
            );

        if (!json.contains("decision") ||
            !json["decision"].is_string())
        {
            throw std::invalid_argument(
                "LLM response missing decision"
            );
        }

        if (!json.contains("confidence") ||
            !json["confidence"].is_string())
        {
            throw std::invalid_argument(
                "LLM response missing confidence"
            );
        }

        if (!json.contains("financial_impact") ||
            !json["financial_impact"].is_number_integer())
        {
            throw std::invalid_argument(
                "LLM response missing financial impact"
            );
        }

        const DecisionType decisionType =
            parseDecisionType(
                json["decision"].get<std::string>()
            );

        const DecisionConfidence confidence =
            parseConfidence(
                json["confidence"].get<std::string>()
            );

        const Money financialImpact =
            json["financial_impact"].get<Money>();

        if (financialImpact < 0)
            throw std::invalid_argument(
                "Financial impact cannot be negative"
            );

        InvestigationDecision decision("DEC-LLM");

        decision.setType(decisionType);
        decision.setConfidence(confidence);
        decision.setFinancialImpact(financialImpact);

        if (json.contains("rationale") &&
            json["rationale"].is_string())
        {
            decision.setRationale(
                json["rationale"].get<std::string>()
            );
        }
        else if (json.contains("reasoning") &&
                 json["reasoning"].is_string())
        {
            decision.setRationale(
                json["reasoning"].get<std::string>()
            );
        }

        if (json.contains("evidence_ids") &&
            json["evidence_ids"].is_array())
        {
            for (const auto& evidenceId :
                 json["evidence_ids"])
            {
                if (evidenceId.is_string())
                {
                    decision.addEvidence(
                        evidenceId.get<std::string>()
                    );
                }
            }
        }

        InvestigationResponse result("RESP-LLM");

        result.setDecision(std::move(decision));

        if (json.contains("reasoning") &&
            json["reasoning"].is_string())
        {
            result.setReasoning(
                json["reasoning"].get<std::string>()
            );
        }

        if (json.contains("missing_evidence"))
        {
            if (json["missing_evidence"].is_string())
            {
                result.setMissingEvidence(
                    json["missing_evidence"].get<std::string>()
                );
            }
            else if (json["missing_evidence"].is_array())
            {
                std::string missingEvidence;

                for (const auto& item :
                     json["missing_evidence"])
                {
                    if (!item.is_string())
                        continue;

                    if (!missingEvidence.empty())
                        missingEvidence += "; ";

                    missingEvidence +=
                        item.get<std::string>();
                }

                result.setMissingEvidence(
                    std::move(missingEvidence)
                );
            }
        }

        if (json.contains("requires_more_evidence") &&
            json["requires_more_evidence"].is_boolean())
        {
            result.setRequiresMoreEvidence(
                json["requires_more_evidence"].get<bool>()
            );
        }

        if (json.contains("requested_tools") &&
            json["requested_tools"].is_array())
        {
            for (const auto& tool :
                 json["requested_tools"])
            {
                if (!tool.is_object())
                    continue;

                std::string toolName;
                std::string input;

                if (tool.contains("tool_name") &&
                    tool["tool_name"].is_string())
                {
                    toolName =
                        tool["tool_name"].get<std::string>();
                }
                else if (tool.contains("tool") &&
                         tool["tool"].is_string())
                {
                    toolName =
                        tool["tool"].get<std::string>();
                }

                if (tool.contains("input") &&
                    tool["input"].is_string())
                {
                    input =
                        tool["input"].get<std::string>();
                }

                if (toolName.empty() || input.empty())
                    continue;

                InvestigationToolRequest toolRequest(
                    std::move(toolName),
                    std::move(input)
                );

                result.addRequestedToolCall(
                    std::move(toolRequest)
                );
            }
        }

        if (json.contains("hypotheses") &&
            json["hypotheses"].is_array())
        {
            for (const auto& item :
                 json["hypotheses"])
            {
                if (!item.is_object())
                    continue;

                if (!item.contains("id") ||
                    !item["id"].is_string())
                {
                    continue;
                }

                InvestigationHypothesis hypothesis(
                    item["id"].get<std::string>()
                );

                if (item.contains("description") &&
                    item["description"].is_string())
                {
                    hypothesis.setDescription(
                        item["description"].get<std::string>()
                    );
                }

                if (item.contains("confidence_score") &&
                    item["confidence_score"].is_number_integer())
                {
                    hypothesis.setConfidenceScore(
                        item["confidence_score"].get<int>()
                    );
                }

                if (item.contains("estimated_impact") &&
                    item["estimated_impact"].is_number_integer())
                {
                    const Money impact =
                        item["estimated_impact"].get<Money>();

                    if (impact < 0)
                        throw std::invalid_argument(
                            "Hypothesis impact cannot be negative"
                        );

                    hypothesis.setEstimatedImpact(impact);
                }

                if (item.contains("evidence_ids") &&
                    item["evidence_ids"].is_array())
                {
                    for (const auto& evidenceId :
                         item["evidence_ids"])
                    {
                        if (evidenceId.is_string())
                        {
                            hypothesis.addEvidence(
                                evidenceId.get<std::string>()
                            );
                        }
                    }
                }

                if (item.contains("status"))
                {
                    hypothesis.setStatus(
                        parseHypothesisStatus(
                            item["status"]
                        )
                    );
                }

                result.addHypothesis(
                    std::move(hypothesis)
                );
            }
        }

        return result;
    }
}