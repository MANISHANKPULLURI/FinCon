#pragma once
#include "common/Money.h"
#include "common/Timestamp.h"
#include <string>

namespace fincon{
    enum class RefundStatus{
        Intiated,
        Processed,
        Failed,
    };

    struct Refund{
        std::string id;
        std::string merchantId;
        std::string paymentId;
        Money amount{0};
        RefundStatus status{RefundStatus::Intiated};
        Timestamp createdAt;
        Timestamp processedAt;
    };
}