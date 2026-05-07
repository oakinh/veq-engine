#pragma once

#include <veq/storage/batch.hpp>
#include <veq/storage/column.hpp>


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
            for (std::size_t row_idx { start_row }; row_idx < size + start_row; ++row_idx) {
                if (compare(target_column.data[row_idx], value)) {
                    // selection stores global table indexes
                    selection_[i] = row_idx;
                    ++i; // Only increment if data is selected
                }
            }

            return {
                .columns=batch.columns,
                .selection=selection_,
                .size=i
            };

        }
    private:
        // Buffer, list of indexes
        std::array<std::size_t, MAX_BATCH_SIZE> selection_ {};
    };
}
