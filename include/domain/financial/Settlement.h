#pragma once
#include "common/Money.h"
#include "common/Timestamp.h"
#include <string>
#include <vector>


namespace fincon{
    enum class SettlementStatus{
        Created,
        Processed,
        Settled,
        Failed,
    };
    struct Settlement{
        std::string id;
        std::string merchantId;
        std::vector<std::string> paymentIds;
        Money grossAmount{0};
        Money refundAmount{0};
        Money feeAmount{0};
        Money netAmount{0};
        SettlementStatus status{SettlementStatus::Created};
        Timestamp createdAt;
        Timestamp settledAt;
       
    };
}