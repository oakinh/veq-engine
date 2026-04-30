#pragma once

#include <array>
#include <span>
#include <veq/storage/table.hpp>

namespace veq {
    constexpr std::size_t MAX_BATCH_SIZE = 1024;

    struct ColumnBatch {
        std::span<const ColumnView> columns {};
        std::size_t start_row {};
        std::size_t size {};
    };

    struct SelectedBatch {
        std::span<const ColumnView> columns {};
        std::span<const std::size_t> selection {};
        std::size_t size {};
    };
}
