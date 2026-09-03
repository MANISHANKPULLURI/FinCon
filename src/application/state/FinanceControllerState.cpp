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
    }
}