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
        std::size_t maxIterations)
        : agent_(agent),
          toolRegistry_(toolRegistry),
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
        std::unordered_set<std::string> failedToolRequests;

        for (std::size_t iteration = 0;
             iteration < maxIterations_;
             ++iteration)
        {
            InvestigationResponse response =
                agent_.investigate(request);

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

                const InvestigationTool* tool =
                    toolRegistry_.get(
                        requestedTool.toolName()
                    );

                if (tool == nullptr)
                {
                    if (failedToolRequests.contains(requestKey))
                        continue;

                    failedToolRequests.insert(requestKey);

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
            {
                return response;
            }
        }

        throw std::runtime_error(
            "Maximum investigation agent iterations exceeded"
        );
    }
}