#include "application/investigation/InvestigationAgentOrchestrator.h"

#include "application/investigation/InvestigationTool.h"
#include "domain/investigation/InvestigationToolCall.h"

#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>

namespace fincon
{
    InvestigationAgentOrchestrator::InvestigationAgentOrchestrator(
        const LLMInvestigationAgent& agent,
        const InvestigationToolRegistry& toolRegistry,
        const InvestigationToolRequestValidator& toolRequestValidator,
        const InvestigationResponseValidator& responseValidator,
        std::size_t maxIterations)
        : agent_(agent),
          toolRegistry_(toolRegistry),
          toolRequestValidator_(toolRequestValidator),
          responseValidator_(responseValidator),
          maxIterations_(maxIterations)
    {
        if (maxIterations_ == 0)
            throw std::invalid_argument(
                "Maximum investigation iterations must be greater than zero"
            );
    }

    InvestigationResponse
    InvestigationAgentOrchestrator::investigate(
        InvestigationRequest request) const
    {
        std::unordered_set<std::string> processedRequests;

        for (std::size_t iteration = 0;
             iteration < maxIterations_;
             ++iteration)
        {
            InvestigationResponse response =
                agent_.investigate(request);

            std::string validationError;

            if (!responseValidator_.validate(
                    response,
                    request,
                    validationError))
            {
                throw std::runtime_error(
                    "Invalid LLM investigation response: " +
                    validationError
                );
            }

            if (response.requestedToolCalls().empty())
                return response;

            bool executedAnyTool = false;

            for (const InvestigationToolRequest& requestedTool :
                 response.requestedToolCalls())
            {
                const std::string requestKey =
                    requestedTool.toolName() +
                    "|" +
                    requestedTool.input();

                if (processedRequests.contains(requestKey))
                    continue;

                processedRequests.insert(requestKey);

                InvestigationToolCall toolRequest(
                    "TC-LLM-" +
                    std::to_string(iteration) +
                    "-" +
                    std::to_string(request.toolCalls().size())
                );

                toolRequest.setToolName(
                    requestedTool.toolName()
                );

                toolRequest.setInput(
                    requestedTool.input()
                );

                std::string toolValidationError;

                if (!toolRequestValidator_.validate(
                        requestedTool,
                        toolValidationError))
                {
                    toolRequest.setStatus(
                        ToolCallStatus::Failed
                    );

                    toolRequest.setResult(
                        toolValidationError
                    );

                    request.addToolCall(
                        std::move(toolRequest)
                    );

                    continue;
                }

                const InvestigationTool* tool =
                    toolRegistry_.get(
                        requestedTool.toolName()
                    );

                if (tool == nullptr)
                {
                    toolRequest.setStatus(
                        ToolCallStatus::Failed
                    );

                    toolRequest.setResult(
                        "Investigation tool not registered: " +
                        requestedTool.toolName()
                    );

                    request.addToolCall(
                        std::move(toolRequest)
                    );

                    continue;
                }

                InvestigationToolCall result =
                    tool->execute(toolRequest);

                request.addToolCall(
                    std::move(result)
                );

                executedAnyTool = true;
            }

            if (!executedAnyTool)
                return response;
        }

        throw std::runtime_error(
            "Maximum investigation agent iterations exceeded"
        );
    }
}