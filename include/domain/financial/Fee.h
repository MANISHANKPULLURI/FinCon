#pragma once
#include "common/Money.h"
#include "common/Timestamp.h"
#include <string>


namespace fincon{
    enum class FeeType{
        Processing,
        Platform,
        Tax,
        Other
    };
    struct Fee{
        std::string id;
        std::string merchantId;
        std::string paymentId;
        FeeType type{FeeType::Processing};
        Money amount{0};
        Timestamp createdAt;
    };
}