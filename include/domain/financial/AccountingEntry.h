#pragma once
#include "common/Money.h"
#include "common/Timestamp.h"
#include <string>

namespace fincon{
    enum class AccountingEntryType{
        Debit,
        Credit,
    };

    struct AccountingEntry{
        std::string id;
        std::string merchantId;
        std::string reference; 
        Money amount{0};
        AccountingEntryType type{AccountingEntryType::Credit};
       
        Timestamp createdAt;
    };
}