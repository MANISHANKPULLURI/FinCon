#pragma once

#include "domain/incident/Incident.h"
#include "domain/investigation/Investigation.h"

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace fincon
{
    class FinanceControllerState
    {
    public:
        void setIncidents(
            std::vector<Incident> incidents
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

    private:
        mutable std::mutex mutex_;

        std::vector<Incident> incidents_;

        std::unordered_map<
            std::string,
            Investigation
        > investigations_;

        bool processingComplete_ = false;
    };
}