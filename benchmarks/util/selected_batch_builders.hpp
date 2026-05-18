#pragma once

#include <ranges>
#include <veq/storage/batch.hpp>
#include <veq/storage/table/table.hpp>
#include <veq/execution/filter.hpp>



namespace veq::bench {

    struct OwnedSelectedBatch {
        std::span<const ColumnView> columns{};
        std::vector<std::size_t> owned_selection{}; // Global indexes
        std::size_t size{};                       // Size of selection
    };

    template <typename T>
    std::vector<OwnedSelectedBatch> buildOwnedSelectedBatches(
        const std::vector<ColumnBatch>& column_batches,
        ColumnView target_column,
        FilterOperation<T> filter_op
        ) {

        std::vector<OwnedSelectedBatch> owned_selected_batches {};
        Filter filter {};

        for (const auto& batch : column_batches) {
            const auto& [columns, selection, size] { filter.apply(batch, target_column, filter_op) };
            std::vector<std::size_t> owned_selection (
            selection.begin(),
            selection.begin() + size);

            owned_selected_batches.emplace_back(
                columns,
                std::move(selection),
                size);
        }

        return owned_selected_batches;
    }

    std::vector<SelectedBatch> convertOwnedSelectedBatches(const std::vector<OwnedSelectedBatch>& owned_batches) {
        std::vector<SelectedBatch> selected_batches {};

        for (const auto& batch : owned_batches) {
            const auto& [columns, owned_selection, size] = batch;

            selected_batches.emplace_back(
                columns,
                owned_selection,
                size);
        }

        return selected_batches;
    }
}

