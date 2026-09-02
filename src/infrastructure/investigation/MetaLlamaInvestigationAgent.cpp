#include "infrastructure/investigation/MetaLlamaInvestigationAgent.h"

#include <nlohmann/json.hpp>

#include <stdexcept>
#include <string>

namespace fincon
{
    namespace
    {
        nlohmann::json buildIncidentJson(
            const Incident& incident)
        {
            return {
                {"id", incident.id()},
                {"type", static_cast<int>(incident.type())},
                {"status", static_cast<int>(incident.status())},
                {"financial_impact", incident.financialImpact()},
                {"finding_ids", incident.findingIds()},
                {"entity_ids", incident.entityIds()}
            };
        }

        nlohmann::json buildEvidenceJson(
            const InvestigationRequest& request)
        {
            nlohmann::json evidence = nlohmann::json::array();

            for (const auto& item : request.evidence())
            {
                evidence.push_back({
                    {"id", item.id()},
                    {"type", static_cast<int>(item.type())},
                    {"strength", static_cast<int>(item.strength())},
                    {"source_id", item.sourceId()},
                    {"description", item.description()},
                    {"financial_impact", item.financialImpact()}
                });
            }

            return evidence;
        }

        nlohmann::json buildHypothesesJson(
            const InvestigationRequest& request)
        {
            nlohmann::json hypotheses =
                nlohmann::json::array();

            for (const auto& item : request.hypotheses())
            {
                hypotheses.push_back({
                    {"id", item.id()},
                    {"description", item.description()},
                    {"status", static_cast<int>(item.status())},
                    {"confidence_score", item.confidenceScore()},
                    {"estimated_impact", item.estimatedImpact()},
                    {"evidence_ids", item.evidenceIds()}
                });
            }

            return hypotheses;
        }

        nlohmann::json buildToolCallsJson(
            const InvestigationRequest& request)
        {
            nlohmann::json toolCalls =
                nlohmann::json::array();

            for (const auto& item : request.toolCalls())
            {
                toolCalls.push_back({
                    {"id", item.id()},
                    {"tool_name", item.toolName()},
                    {"input", item.input()},
                    {"result", item.result()},
                    {"status", static_cast<int>(item.status())}
                });
            }

            return toolCalls;
        }
    }

    MetaLlamaInvestigationAgent::MetaLlamaInvestigationAgent(
        const LLMProvider& llmProvider,
        const InvestigationResponseParser& responseParser)
        : llmProvider_(llmProvider),
          responseParser_(responseParser)
    {
    }

    InvestigationResponse
    MetaLlamaInvestigationAgent::investigate(
        const InvestigationRequest& request) const
    {
        if (request.incident() == nullptr)
        {
            throw std::invalid_argument(
                "Investigation request has no incident"
            );
        }

        const nlohmann::json investigationRequest = {
            {
                "role",
                "You are FinCon's financial investigation agent."
            },
            {
                "objective",
                "Investigate the detected financial exception, "
                "identify the most likely root cause, determine "
                "whether additional evidence is required, and "
                "propose a controlled financial decision."
            },
            {
                "rules",
                {
                    "Do not invent financial facts.",
                    "Use only supplied evidence.",
                    "Do not change financial records.",
                    "Financial impact must be expressed in paise.",
                    "Request additional evidence when the case "
                    "cannot be safely resolved."
                }
            },
            {
                "required_response",
                {
                    "decision",
                    "confidence",
                    "financial_impact",
                    "rationale",
                    "reasoning",
                    "evidence_ids",
                    "missing_evidence",
                    "requires_more_evidence",
                    "requested_tools",
                    "hypotheses"
                }
            },
            {
                "decision_values",
                {
                    "AUTO_RESOLVE",
                    "HUMAN_REVIEW",
                    "REQUEST_MORE_EVIDENCE",
                    "UNRESOLVED"
                }
            },
            {
                "confidence_values",
                {
                    "LOW",
                    "MEDIUM",
                    "HIGH"
                }
            },
            {
                "incident",
                buildIncidentJson(*request.incident())
            },
            {
                "evidence",
                buildEvidenceJson(request)
            },
            {
                "hypotheses",
                buildHypothesesJson(request)
            },
            {
                "tool_calls",
                buildToolCallsJson(request)
            }
        };

        const std::string llmResponse =
            llmProvider_.generate(
                investigationRequest.dump()
            );

        return responseParser_.parse(llmResponse);
    }
}