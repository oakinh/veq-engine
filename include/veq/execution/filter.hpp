#pragma once

#include <veq/storage/batch.hpp>
#include <veq/storage/column.hpp>
#include <cassert>


namespace veq {
    template <typename Compare>
    struct FilterOperation {
        Compare compare {};
        std::uint64_t value {};
    };

    class Filter {
    public:
        Filter() = default;

        template <typename Compare>
        SelectedBatch apply(const ColumnBatch& batch, const ColumnView target_column, const FilterOperation<Compare> operation) {
            const auto& [compare, value] = operation;
            const auto& [columns, start_row, size] = batch;

            std::size_t i {};
            const std::size_t end_row { size + start_row };
            for (std::size_t row_idx { start_row }; row_idx < end_row; ++row_idx) {
                if (compare(target_column.data[row_idx], value)) {
                    assert(size <= MAX_BATCH_SIZE);
                    // selection stores global table indexes
                    selection_[i] = row_idx;
                    ++i; // Only increment if data is selected
                }
            }

            return {
                .columns = batch.columns,
                .selection = std::span<const std::size_t>{ selection_.data(), i },
                .size = i
            };

        }
    private:
        // Buffer, list of indexes
        std::array<std::size_t, MAX_BATCH_SIZE> selection_ {};
    };
}
