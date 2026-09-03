#include "application/state/FinanceControllerState.h"

#include <utility>

namespace fincon
{
    void FinanceControllerState::setIncidents(
        std::vector<Incident> incidents
    )
    {
        std::lock_guard<std::mutex> lock(mutex_);

        incidents_ = std::move(incidents);
    }

    void FinanceControllerState::addIncidents(
        const std::vector<Incident>& incidents
    )
    {
        std::lock_guard<std::mutex> lock(mutex_);
        incidents_.insert(incidents_.end(), incidents.begin(), incidents.end());
    }

    void FinanceControllerState::setInvestigation(
        Investigation investigation
    )
    {
        std::lock_guard<std::mutex> lock(mutex_);

        investigations_[investigation.id()] =
            std::move(investigation);
    }

    std::vector<Incident>
    FinanceControllerState::getIncidents() const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        return incidents_;
    }

    std::vector<Investigation>
    FinanceControllerState::getInvestigations() const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        std::vector<Investigation> result;

        result.reserve(
            investigations_.size()
        );

        for (const auto& [id, investigation] :
             investigations_)
        {
            result.push_back(investigation);
        }

        return result;
    }

    Investigation FinanceControllerState::getInvestigation(
    const std::string& investigationId
) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    const auto iterator =
        investigations_.find(
            investigationId
        );

    if (iterator == investigations_.end())
    {
        return Investigation{};
    }

    return iterator->second;
}

    std::size_t
    FinanceControllerState::completedInvestigations() const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        std::size_t count = 0;

        for (const auto& [id, investigation] :
             investigations_)
        {
            if (
                investigation.status() ==
                InvestigationStatus::Completed
            )
            {
                ++count;
            }
        }

        return count;
    }

    std::size_t
    FinanceControllerState::totalInvestigations() const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        return investigations_.size();
    }

    bool
    FinanceControllerState::processingComplete() const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        return processingComplete_;
    }

    void FinanceControllerState::setProcessingComplete(
        bool complete
    )
    {
        std::lock_guard<std::mutex> lock(mutex_);

        processingComplete_ = complete;
        processingStatus_ = complete ? "completed" : "running";
    }

    void FinanceControllerState::setProcessingStatus(
        std::string status,
        std::string error
    )
    {
        std::lock_guard<std::mutex> lock(mutex_);

        processingStatus_ = std::move(status);
        error_ = std::move(error);
        processingComplete_ = processingStatus_ == "completed";
    }

    void FinanceControllerState::setTotalRecords(
        std::size_t totalRecords
    )
    {
        std::lock_guard<std::mutex> lock(mutex_);

        totalRecords_ = totalRecords;
    }

    void FinanceControllerState::setEvaluation(
        Phase2EvaluationResult result
    )
    {
        std::lock_guard<std::mutex> lock(mutex_);

        evaluation_ = std::move(result);
    }

    bool FinanceControllerState::acceptBatch(
        const std::string& batchId,
        std::size_t recordCount)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (batchIds_.contains(batchId))
            return false;
        batchIds_[batchId] = true;
        ++messagesReceived_;
        ++messagesQueued_;
        recordsReceived_ += recordCount;
        paymentsReceived_ += recordCount;
        totalRecords_ += recordCount;
        return true;
    }

    void FinanceControllerState::markBatchProcessed(
        std::size_t recordCount)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (messagesQueued_ > 0)
            --messagesQueued_;
        ++messagesProcessed_;
        recordsProcessed_ += recordCount;
        paymentsProcessed_ += recordCount;
        if (recordsProcessed_ > incidents_.size())
            matchedCount_ = recordsProcessed_ - incidents_.size();
        else
            matchedCount_ = 0;
    }

    void FinanceControllerState::setInitialProcessedRecords(
        std::size_t recordCount)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        recordsProcessed_ = recordCount;
        recordsReceived_ = std::max(recordsReceived_, recordCount);
        paymentsReceived_ = std::max(paymentsReceived_, recordCount);
        paymentsProcessed_ = std::max(paymentsProcessed_, recordCount);
        totalRecords_ = std::max(totalRecords_, recordCount);
        recordsReceived_ = std::max(recordsReceived_, totalRecords_);
        if (recordCount > incidents_.size())
            matchedCount_ = recordCount - incidents_.size();
    }

    void FinanceControllerState::setActiveWorkers(std::size_t count)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        activeWorkers_ = count;
    }

    void FinanceControllerState::emitEvent(const std::string& type, const std::string& detail)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        Event e;
        e.type = type;
        e.detail = detail;
        e.timestamp = std::to_string(events_.size() + 1);
        events_.push_back(std::move(e));
        if (events_.size() > 200)
            events_.erase(events_.begin());
    }

    FinanceControllerState::Snapshot
    FinanceControllerState::snapshot() const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        Snapshot result;
        result.incidents = incidents_;
        result.processingComplete = processingComplete_;
        result.processingStatus = processingStatus_;
        result.totalRecords = totalRecords_;
        result.error = error_;
        result.evaluation = evaluation_;
        result.messagesReceived = messagesReceived_;
        result.messagesQueued = messagesQueued_;
        result.messagesProcessed = messagesProcessed_;
        result.recordsReceived = recordsReceived_;
        result.recordsProcessed = recordsProcessed_;
        result.paymentsReceived = paymentsReceived_;
        result.paymentsProcessed = paymentsProcessed_;
        result.matchedCount = matchedCount_;
        result.activeWorkers = activeWorkers_;
        result.recentEvents = events_;

        for (const auto& [id, investigation] : investigations_)
        {
            result.investigations.push_back(investigation);

            if (investigation.status() == InvestigationStatus::Completed)
                ++result.completedInvestigations;
        }

        return result;
    }
}