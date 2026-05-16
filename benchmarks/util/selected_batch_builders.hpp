#pragma once

#include <veq/storage/batch.hpp>
#include <veq/storage/table/table.hpp>
#include <veq/execution/filter.hpp>

namespace veq::bench {

    template <typename T>
    std::vector<SelectedBatch> buildSelectedBatches(
        const std::vector<ColumnBatch> column_batches,
        ColumnView target_column,
        FilterOperation<T> filter_op
        ) {

        std::vector<SelectedBatch> selected_batches {};
        Filter filter {};

        for (const auto& batch : column_batches) {
            selected_batches.push_back(filter.apply(batch, target_column, filter_op));
        }

        return selected_batches;
    }
}

