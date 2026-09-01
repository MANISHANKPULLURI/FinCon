#pragma once
#include "common/Money.h"
#include "common/Timestamp.h"
#include <string>


namespace fincon{
    enum class PaymentStatus{
        Created,
        Authorized,
        Captured,
        Failed,
        PartiallyRefunded,
        Refunded,
    };
    struct Payment{
        std::string id;
        std::string merchantId;
        std::string orderId;
        Money amount{0};
        PaymentStatus status{PaymentStatus::Created};
        Timestamp createdAt;
    };
}
