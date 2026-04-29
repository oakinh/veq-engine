#pragma once

#include <vector>
#include <cstdint>

namespace veq {
    using Column = std::vector<std::uint64_t>;
    struct ColumnView {
        const std::uint64_t* data;
    };
}