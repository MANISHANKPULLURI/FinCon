#include "application/controller/FinanceControllerApi.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <stdexcept>

namespace fincon
{
    namespace
    {
        using Json = nlohmann::json;

        std::string jsonError(
            const std::string& message
        )
        {
            return Json{
                {"error", message}
            }.dump();
        }

        std::string statusName(InvestigationStatus status)
        {
            switch (status)
            {
            case InvestigationStatus::Pending: return "Pending";
            case InvestigationStatus::InProgress: return "InProgress";
            case InvestigationStatus::EvidenceCollected: return "EvidenceCollected";
            case InvestigationStatus::DecisionReady: return "DecisionReady";
            case InvestigationStatus::Completed: return "Completed";
            case InvestigationStatus::Failed: return "Failed";
            }
            return "Unknown";
        }

        std::string outcomeName(InvestigationOutcome outcome)
        {
            switch (outcome)
            {
            case InvestigationOutcome::AutoResolve: return "AutoResolve";
            case InvestigationOutcome::HumanReview: return "HumanReview";
            case InvestigationOutcome::RequestMoreEvidence: return "RequestMoreEvidence";
            case InvestigationOutcome::Unresolved: return "Unresolved";
            case InvestigationOutcome::Unknown: return "Unknown";
            }
            return "Unknown";
        }

        std::string confidenceName(ConfidenceLevel confidence)
        {
            switch (confidence)
            {
            case ConfidenceLevel::Low: return "Low";
            case ConfidenceLevel::Medium: return "Medium";
            case ConfidenceLevel::High: return "High";
            case ConfidenceLevel::Unknown: return "Unknown";
            }
            return "Unknown";
        }

        std::string recommendationActionName(RecommendationAction action)
        {
            switch (action)
            {
            case RecommendationAction::None: return "None";
            case RecommendationAction::MarkResolved: return "MarkResolved";
            case RecommendationAction::RequestEvidence: return "RequestEvidence";
            case RecommendationAction::EscalateToFinance: return "EscalateToFinance";
            case RecommendationAction::ApplyFinancialAdjustment: return "ApplyFinancialAdjustment";
            }
            return "None";
        }

        Json investigationJson(const Investigation& investigation)
        {
            Json result = {
                {"id", investigation.id()},
                {"incidentId", investigation.incidentId()},
                {"status", statusName(investigation.status())},
                {"outcome", outcomeName(investigation.outcome())},
                {"confidence", confidenceName(investigation.confidence())},
                {"confirmedImpact", investigation.confirmedImpact()},
                {"llmEscalated", investigation.llmEscalated()},
                {"evidenceIds", investigation.evidenceIds()},
                {"hypothesisIds", investigation.hypothesisIds()},
                {"toolCallIds", investigation.toolCallIds()}
            };

            result["evidence"] = Json::array();
            for (const auto& item : investigation.evidence())
            {
                result["evidence"].push_back({
                    {"id", item.id()},
                    {"type", static_cast<int>(item.type())},
                    {"sourceId", item.sourceId()},
                    {"description", item.description()},
                    {"financialImpact", item.financialImpact()},
                    {"strength", static_cast<int>(item.strength())}
                });
            }

            result["hypotheses"] = Json::array();
            for (const auto& item : investigation.hypotheses())
            {
                result["hypotheses"].push_back({
                    {"id", item.id()},
                    {"description", item.description()},
                    {"status", static_cast<int>(item.status())},
                    {"confidenceScore", item.confidenceScore()},
                    {"estimatedImpact", item.estimatedImpact()},
                    {"evidenceIds", item.evidenceIds()}
                });
            }

            result["toolCalls"] = Json::array();
            for (const auto& item : investigation.toolCalls())
            {
                result["toolCalls"].push_back({
                    {"id", item.id()},
                    {"tool", item.toolName()},
                    {"input", item.input()},
                    {"status", static_cast<int>(item.status())},
                    {"result", item.result()}
                });
            }

            if (const InvestigationRecommendation* recommendation =
                    investigation.recommendation())
            {
                result["recommendation"] = {
                    {"id", recommendation->id()},
                    {"action", recommendationActionName(recommendation->action())},
                    {"confidence", confidenceName(
                        recommendation->confidence() == DecisionConfidence::Low
                            ? ConfidenceLevel::Low
                            : recommendation->confidence() == DecisionConfidence::Medium
                                ? ConfidenceLevel::Medium
                                : ConfidenceLevel::High)},
                    {"rationale", recommendation->rationale()},
                    {"financialImpact", recommendation->financialImpact()},
                    {"evidenceIds", recommendation->evidenceIds()}
                };
            }

            return result;
        }

        Json incidentJson(const Incident& incident)
        {
            return {
                {"id", incident.id()},
                {"status", static_cast<int>(incident.status())},
                {"type", static_cast<int>(incident.type())},
                {"financialImpact", incident.financialImpact()},
                {"findingIds", incident.findingIds()},
                {"entityIds", incident.entityIds()}
            };
        }
    }

    FinanceControllerApi::FinanceControllerApi(
        FinanceControllerFacade& facade,
        MessageQueue<FinancialDataBatch>& queue,
        FinanceControllerState& state
    )
        : facade_(facade), queue_(queue), state_(state)
    {
    }

   void FinanceControllerApi::registerRoutes(
    HttpServer& server
)
{
    server.registerHandler(
        "GET",
        "/health",
        [this](const HttpRequest& request)
        {
            return health(request);
        }
    );

    server.registerHandler("GET", "/api/dashboard", [this](const HttpRequest& request)
    {
        return dashboard(request);
    });
    server.registerHandler("GET", "/api/incidents", [this](const HttpRequest& request)
    {
        return incidents(request);
    });
    server.registerHandler("GET", "/api/investigations", [this](const HttpRequest& request)
    {
        return investigations(request);
    });
    server.registerHandler("GET", "/api/investigations/{id}", [this](const HttpRequest& request)
    {
        return investigation(request);
    });
    server.registerHandler("GET", "/api/investigations/{id}/audit", [this](const HttpRequest& request)
    {
        return audit(request);
    });
    server.registerHandler("GET", "/api/events", [this](const HttpRequest& request)
    {
        return events(request);
    });
    server.registerHandler("GET", "/api/evaluation", [this](const HttpRequest& request)
    {
        return evaluation(request);
    });

    server.registerHandler(
        "POST",
        "/api/investigations",
        [this](const HttpRequest& request)
        {
            return investigate(request);
        }
    );
    server.registerHandler(
        "POST",
        "/api/ingest",
        [this](const HttpRequest& request)
        {
            return ingest(request);
        }
    );
}

    HttpResponse FinanceControllerApi::dashboard(const HttpRequest&) const
    {
        const auto state = facade_.snapshot();
        std::size_t autoResolved = 0;
        std::size_t humanReview = 0;
        std::size_t moreEvidence = 0;
        std::size_t unresolved = 0;
        std::size_t unknownType = 0;
        std::int64_t exposure = 0;
        for (const auto& incident : state.incidents) { exposure += incident.financialImpact(); if (incident.type() == IncidentType::Unknown) ++unknownType; }
        for (const auto& item : state.investigations)
        {
            switch (item.outcome())
            {
            case InvestigationOutcome::AutoResolve: ++autoResolved; break;
            case InvestigationOutcome::HumanReview: ++humanReview; break;
            case InvestigationOutcome::RequestMoreEvidence: ++moreEvidence; break;
            case InvestigationOutcome::Unresolved: ++unresolved; break;
            case InvestigationOutcome::Unknown: ++unresolved; break;
            }
        }
        std::size_t queueDepth = state.messagesQueued;
        std::size_t matched = state.matchedCount;
        if (matched == 0 && state.recordsProcessed > state.incidents.size()) matched = state.recordsProcessed - state.incidents.size();
        return {200, "application/json", Json{
            {"totalFinancialRecords", state.totalRecords},
            {"totalIncidents", state.incidents.size()},
            {"totalInvestigations", state.investigations.size()},
            {"completedInvestigations", state.completedInvestigations},
            {"pendingInvestigations", state.investigations.size() - state.completedInvestigations},
            {"autoResolvedCases", autoResolved},
            {"humanReviewCases", humanReview},
            {"requestMoreEvidenceCases", moreEvidence},
            {"unresolvedCases", unresolved},
            {"unknownCases", unknownType},
            {"llmEscalations", std::count_if(state.investigations.begin(), state.investigations.end(), [](const Investigation& item) { return item.llmEscalated(); })},
            {"totalFinancialExposure", exposure},
            {"processingStatus", state.processingStatus},
            {"processingComplete", state.processingComplete},
            {"processingProgress", state.completedInvestigations},
            {"processingTotal", state.investigations.size()},
            {"messagesReceived", state.messagesReceived},
            {"messagesQueued", queueDepth},
            {"queueDepth", queueDepth},
            {"messagesProcessed", state.messagesProcessed},
            {"recordsReceived", state.recordsReceived},
            {"recordsProcessed", state.recordsProcessed},
            {"transactionsReceived", state.recordsReceived},
            {"transactionsProcessed", state.recordsProcessed},
            {"matchedTransactions", matched},
            {"exceptions", state.incidents.size()},
            {"investigations", state.investigations.size()},
            {"paymentsReceived", state.paymentsReceived},
            {"paymentsProcessed", state.paymentsProcessed},
            {"activeWorkers", state.activeWorkers},
            {"error", state.error}
        }.dump()};
    }

    HttpResponse FinanceControllerApi::incidents(const HttpRequest&) const
    {
        const auto state = facade_.snapshot();
        Json result = Json::array();
        for (const auto& incident : state.incidents)
            result.push_back(incidentJson(incident));
        return {200, "application/json", result.dump()};
    }

    HttpResponse FinanceControllerApi::investigations(const HttpRequest&) const
    {
        const auto state = facade_.snapshot();
        Json result = Json::array();
        for (const auto& item : state.investigations)
            result.push_back(investigationJson(item));
        return {200, "application/json", result.dump()};
    }

    HttpResponse FinanceControllerApi::investigation(const HttpRequest& request) const
    {
        const std::string prefix = "/api/investigations/";
        const std::string id = request.path.substr(prefix.size());
        if (id.empty() || id.ends_with("/audit"))
            return {400, "application/json", jsonError("Invalid investigation id.")};
        const auto state = facade_.snapshot();
        for (const auto& item : state.investigations)
        {
            if (item.id() == id)
                return {200, "application/json", investigationJson(item).dump()};
        }
        return {404, "application/json", jsonError("Investigation not found.")};
    }

    HttpResponse FinanceControllerApi::events(const HttpRequest&) const
    {
        const auto state = facade_.snapshot();
        std::string body;
        for (const auto& ev : state.recentEvents)
        {
            Json p = {{"type", ev.type}, {"detail", ev.detail}, {"timestamp", ev.timestamp}};
            body += "event: " + ev.type + "\ndata: " + p.dump() + "\n\n";
        }
        Json snap = {
            {"type", "state_snapshot"},
            {"processingStatus", state.processingStatus},
            {"completedInvestigations", state.completedInvestigations},
            {"totalInvestigations", state.investigations.size()},
            {"totalIncidents", state.incidents.size()},
            {"messagesQueued", state.messagesQueued},
            {"queueDepth", state.messagesQueued},
            {"recordsProcessed", state.recordsProcessed},
            {"activeWorkers", state.activeWorkers}
        };
        body += "event: state_snapshot\ndata: " + snap.dump() + "\n\n";
        if (body.empty()) body = "event: state_snapshot\ndata: " + snap.dump() + "\n\n";
        return {200, "text/event-stream", body, "http://127.0.0.1:5173", true};
    }

    HttpResponse FinanceControllerApi::evaluation(const HttpRequest&) const
    {
        const auto state = facade_.snapshot();
        if (!state.evaluation)
            return {503, "application/json", jsonError("Evaluation is not available yet.")};

        const auto& item = *state.evaluation;
        return {200, "application/json", Json{
            {"records", item.records},
            {"injectedExceptions", item.injectedExceptions},
            {"incidents", item.incidents},
            {"investigations", item.investigations},
            {"deterministicResolutions", item.deterministicResolutions},
            {"llmResolutions", item.llmResolutions},
            {"autoResolved", item.autoResolved},
            {"humanReview", item.humanReview},
            {"requestMoreEvidence", item.requestMoreEvidence},
            {"unresolved", item.unresolved},
            {"correctIncidentTypes", item.correctIncidentTypes},
            {"correctOutcomes", item.correctOutcomes},
            {"correctImpacts", item.correctImpacts},
            {"correctLLMEscalations", item.correctLLMEscalations},
            {"correctlyEvaluated", item.correctlyEvaluated},
            {"passed", item.passed}
        }.dump()};
    }

    HttpResponse FinanceControllerApi::health(
        const HttpRequest&
    ) const
    {
        return HttpResponse{
            200,
            "application/json",
            R"({"status":"ok","service":"fincon"})"
        };
    }

    HttpResponse FinanceControllerApi::ingest(
        const HttpRequest& request
    ) const
    {
        try
        {
            const MappingResult mapped =
                mapper_.map(Json::parse(request.body));
            if (!mapped.accepted)
            {
                Json response = {
                    {"accepted", false},
                    {"batchId", mapped.batchId},
                    {"status", mapped.status},
                    {"reason", mapped.reason},
                    {"missingFields", mapped.missingFields}
                };
                return {400, "application/json", response.dump()};
            }

            const std::size_t records = mapped.batch.data.payments.size();
            if (!state_.acceptBatch(mapped.batch.batchId, records))
            {
                return {200, "application/json", Json{
                    {"accepted", true},
                    {"batchId", mapped.batch.batchId},
                    {"status", "DUPLICATE"},
                    {"messageId", "MSG-" + mapped.batch.batchId}
                }.dump()};
            }

            state_.emitEvent("batch_received", mapped.batch.batchId);
            state_.emitEvent("batch_queued", mapped.batch.batchId);
            if (!queue_.push(mapped.batch))
                return {503, "application/json", jsonError("Ingestion queue is shutting down.")};

            return {202, "application/json", Json{
                {"accepted", true},
                {"messageId", "MSG-" + mapped.batch.batchId},
                {"batchId", mapped.batch.batchId},
                {"status", "QUEUED"},
                {"schemaStatus", mapped.status}
            }.dump()};
        }
        catch (const Json::exception&)
        {
            return {400, "application/json", jsonError("Malformed JSON request.")};
        }
        catch (const std::exception&)
        {
            return {400, "application/json", jsonError("Invalid financial batch.")};
        }
    }

    HttpResponse FinanceControllerApi::investigate(
        const HttpRequest& request
    ) const
    {
        try
        {
            if (request.body.empty())
            {
                return HttpResponse{
                    400,
                    "application/json",
                    jsonError(
                        "Request body cannot be empty."
                    )
                };
            }

            const Json body =
                Json::parse(request.body);

            if (!body.contains("incident"))
            {
                return HttpResponse{
                    400,
                    "application/json",
                    jsonError(
                        "Missing incident."
                    )
                };
            }

            const Json incidentJson =
                body.at("incident");

            Incident incident;

            if (incidentJson.contains("id"))
            {
                incident =
                    Incident(
                        incidentJson.at("id")
                            .get<std::string>()
                    );
            }
            else
            {
                return HttpResponse{
                    400,
                    "application/json",
                    jsonError(
                        "Incident id is required."
                    )
                };
            }

            if (incidentJson.contains("financialImpact"))
            {
                incident.setFinancialImpact(
                    incidentJson
                        .at("financialImpact")
                        .get<std::int64_t>()
                );
            }

            const Investigation investigation =
                facade_.investigate(
                    incident
                );

            Json response;

            response["id"] =
                investigation.id();

            response["incidentId"] =
                investigation.incidentId();

            response["confirmedImpact"] =
                investigation.confirmedImpact();

            response["llmEscalated"] =
                investigation.llmEscalated();

            response["evidenceIds"] =
                investigation.evidenceIds();

            response["hypothesisIds"] =
                investigation.hypothesisIds();

            response["toolCallIds"] =
                investigation.toolCallIds();

            return HttpResponse{
                200,
                "application/json",
                response.dump()
            };
        }
        catch (const Json::exception& exception)
        {
            return HttpResponse{
                400,
                "application/json",
                jsonError(
                    exception.what()
                )
            };
        }
        catch (const std::exception&)
        {
            return HttpResponse{
                500,
                "application/json",
                jsonError(
                    "Unable to process investigation request."
                )
            };
        }
    }

    HttpResponse FinanceControllerApi::audit(
        const HttpRequest& request
    ) const
    {
        try
        {
            const std::string prefix =
                "/api/investigations/";

            const std::string suffix =
                "/audit";

            if (
                request.path.size() <=
                prefix.size() + suffix.size()
            )
            {
                return HttpResponse{
                    400,
                    "application/json",
                    jsonError(
                        "Invalid investigation id."
                    )
                };
            }

            const std::size_t idStart =
                prefix.size();

            const std::size_t idLength =
                request.path.size() -
                prefix.size() -
                suffix.size();

            const std::string investigationId =
                request.path.substr(
                    idStart,
                    idLength
                );

            const auto entries =
                facade_.getAuditTrail(
                    investigationId
                );

            Json response =
                Json::array();

            for (const auto& entry : entries)
            {
                response.push_back(
                    {
                        {"id", entry.id()},
                        {"investigationId",
                         entry.investigationId()},
                        {"incidentId",
                         entry.incidentId()},
                        {"actor",
                         entry.actor()},
                        {"description",
                         entry.description()},
                        {"financialImpact",
                         entry.financialImpact()}
                    }
                );
            }

            return HttpResponse{
                200,
                "application/json",
                response.dump()
            };
        }
        catch (const std::exception&)
        {
            return HttpResponse{
                500,
                "application/json",
                jsonError(
                    "Unable to load audit trail."
                )
            };
        }
    }
}