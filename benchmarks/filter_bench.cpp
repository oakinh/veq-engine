#include <benchmark/benchmark.h>
#include <cstddef>
#include <veq/execution/filter.hpp>
#include "util/column_batch_builders.hpp"

namespace veq::bench {
    constexpr std::size_t batch_size { 1024 };

    static void BM_FilterAge(benchmark::State& state) {
        auto row_count { state.range(0) };
        Table table { buildRandomAgeTable(row_count) };
        std::vector<ColumnBatch> batches { buildBatches(table) };


        FilterOperation<std::greater<>> filter_op { std::greater{}, 50 };
        ColumnView target_column { table.age.data() };
        for (auto _ : state) {
            Filter filter {};
            std::size_t total_selected {};

            for (const auto& batch : batches) {
                SelectedBatch selected_batch { filter.apply(batch, target_column, filter_op) };
                total_selected += selected_batch.size;
                benchmark::DoNotOptimize(selected_batch.selection.data());
            }

            benchmark::DoNotOptimize(total_selected);
        }

        state.SetItemsProcessed(state.iterations() * row_count);
        state.SetBytesProcessed(state.iterations() * row_count * sizeof(std::uint64_t));
    }

    BENCHMARK(BM_FilterAge)
    ->RangeMultiplier(2)
    ->Range(batch_size, 1 << 24)
    ->Unit(benchmark::kMicrosecond);
}

