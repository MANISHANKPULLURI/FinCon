#pragma once
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace fincon{
    using Money = std::int64_t;
    constexpr Money kPaisePerRupee = 100;
    inline Money rupeesToPaise(std::int64_t rupees){
        Money out;
        if (__builtin_mul_overflow(rupees, kPaisePerRupee, &out))
            throw std::overflow_error("Money overflow");
        return out;
    }
    inline Money safeAdd(Money a, Money b){
        Money out;
        if (__builtin_add_overflow(a, b, &out))
            throw std::overflow_error("Money overflow");
        return out;
    }
}
