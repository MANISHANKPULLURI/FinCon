#pragma once
#include <cstdint>

namespace fincon{
    using Money = std::int64_t;
    constexpr Money kPaisePerRupee = 100;
    constexpr Money rupeesToPaise(std::int64_t rupees) noexcept{
        return rupees * kPaisePerRupee;
    }
}
