#include "application/controller/FinanceControllerApi.h"

#include <nlohmann/json.hpp>

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
    }

    FinanceControllerApi::FinanceControllerApi(
        FinanceControllerFacade& facade
    )
        : facade_(facade)
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

    server.registerHandler(
        "POST",
        "/api/investigations",
        [this](const HttpRequest& request)
        {
            return investigate(request);
        }
    );
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
        catch (const std::exception& exception)
        {
            return HttpResponse{
                500,
                "application/json",
                jsonError(
                    exception.what()
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
        catch (const std::exception& exception)
        {
            return HttpResponse{
                500,
                "application/json",
                jsonError(
                    exception.what()
                )
            };
        }
    }
}