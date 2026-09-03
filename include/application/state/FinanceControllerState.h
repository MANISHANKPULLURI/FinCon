#pragma once

#include "application/evaluation/Phase2Evaluation.h"
#include "domain/incident/Incident.h"
#include "domain/investigation/Investigation.h"

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace fincon
{
    class FinanceControllerState
    {
    public:
            struct Event
        {
            std::string type;
            std::string detail;
            std::string timestamp;
        };
    struct Snapshot
        {
            std::vector<Incident> incidents;
            std::vector<Investigation> investigations;
            std::size_t totalRecords = 0;
            std::size_t completedInvestigations = 0;
            bool processingComplete = false;
            std::string processingStatus = "starting";
            std::string error;
            std::optional<Phase2EvaluationResult> evaluation;
            std::size_t messagesReceived = 0;
            std::size_t messagesQueued = 0;
            std::size_t messagesProcessed = 0;
            std::size_t recordsReceived = 0;
            std::size_t recordsProcessed = 0;
            std::size_t paymentsReceived = 0;
            std::size_t paymentsProcessed = 0;
            std::size_t matchedCount = 0;
            std::size_t activeWorkers = 1;
            std::vector<Event> recentEvents;
        };

        void setIncidents(
            std::vector<Incident> incidents
        );

        void addIncidents(
            const std::vector<Incident>& incidents
        );

        void setInvestigation(
            Investigation investigation
        );

        std::vector<Incident> getIncidents() const;

        std::vector<Investigation> getInvestigations() const;

        Investigation getInvestigation(
            const std::string& investigationId
        ) const;

        std::size_t completedInvestigations() const;

        std::size_t totalInvestigations() const;

        bool processingComplete() const;

        void setProcessingComplete(
            bool complete
        );

        void setProcessingStatus(
            std::string status,
            std::string error = {}
        );

        void setTotalRecords(
            std::size_t totalRecords
        );

        void setEvaluation(
            Phase2EvaluationResult result
        );

        bool acceptBatch(const std::string& batchId,
                         std::size_t recordCount);

        void markBatchProcessed(std::size_t recordCount);

        void setInitialProcessedRecords(std::size_t recordCount);

        void setActiveWorkers(std::size_t count);

        void emitEvent(const std::string& type, const std::string& detail);

        Snapshot snapshot() const;

    private:
        mutable std::mutex mutex_;

        std::vector<Incident> incidents_;

        std::unordered_map<
            std::string,
            Investigation
        > investigations_;

        bool processingComplete_ = false;
        std::size_t totalRecords_ = 0;
        std::string processingStatus_ = "starting";
        std::string error_;
        std::optional<Phase2EvaluationResult> evaluation_;
        std::size_t messagesReceived_ = 0;
        std::size_t messagesQueued_ = 0;
        std::size_t messagesProcessed_ = 0;
        std::size_t recordsReceived_ = 0;
        std::size_t recordsProcessed_ = 0;
        std::size_t paymentsReceived_ = 0;
        std::size_t paymentsProcessed_ = 0;
        std::size_t matchedCount_ = 0;
        std::size_t activeWorkers_ = 1;
        std::vector<Event> events_;
        std::unordered_map<std::string, bool> batchIds_;
    };
}