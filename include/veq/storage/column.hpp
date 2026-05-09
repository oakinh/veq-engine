#pragma once

#include <cstdint>
#include <vector>
#include <veq/storage/table/schema.hpp>

namespace veq {
using Column = std::vector<std::uint64_t>;
struct ColumnView {
    const std::uint64_t* data;
};
using ColumnViews = std::vector<ColumnView>;
} // namespace veq