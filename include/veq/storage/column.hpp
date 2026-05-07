#pragma once

#include <vector>
#include <cstdint>
#include <veq/storage/table/schema.hpp>


namespace veq {
    using Column = std::vector<std::uint64_t>;
    struct ColumnView {
        const std::uint64_t* data;
    };
    using ColumnViews = std::vector<ColumnView>;
}