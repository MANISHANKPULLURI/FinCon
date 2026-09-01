#pragma once

#include "common/Money.h"
#include "common/Timestamp.h"

#include <string>

namespace fincon {

enum class BankTransactionType {
    Credit,
    Debit
};

struct BankTransaction {
    std::string id;
    std::string merchantId;
    std::string settlementId;
    Money amount{0};
    BankTransactionType type{
        BankTransactionType::Credit
    };
    Timestamp postedAt;
};

}