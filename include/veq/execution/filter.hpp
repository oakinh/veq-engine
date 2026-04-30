#pragma once

#include <veq/storage/batch.hpp>
#include <veq/storage/column.hpp>

#include "scan.hpp"

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
            std::vector<std::size_t> selection {};
            for (std::size_t i { start_row }; i < size; ++i) {
                if (compare(target_column.data[i], value)) {
                    selection.emplace_back(i);
                }
            }

            selection_ = selection;

            return {
                .columns=batch.columns,
                .selection=selection_,
                .size=selection.size()
            };

        }
    private:
        // Buffer, list of indexes
        std::vector<std::size_t> selection_ {};
    };
}
