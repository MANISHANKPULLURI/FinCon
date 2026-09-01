#pragma once
#include "common/Money.h"
#include "common/Timestamp.h"
#include <string>

namespace fincon{
    enum class OrderStatus{
        Created,
        Paid,
        Cancelled,
    };

    struct Order{
        std::string id;
        std::string merchantId;
        Money amount{0};
        OrderStatus status{OrderStatus::Created};
        Timestamp createdAt;
    };

}