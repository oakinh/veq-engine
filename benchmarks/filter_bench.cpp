#include "util/column_batch_builders.hpp"
#include <benchmark/benchmark.h>
#include <cstddef>
#include <veq/execution/filter.hpp>

namespace veq::bench {
// constexpr std::size_t batch_size { 1024 };

static void BM_FilterAge(benchmark::State& state) {
    auto row_count{ static_cast<std::size_t>(state.range(0)) };
    uint64_t compare_value{ static_cast<uint64_t>(state.range(1)) };

    Table table{ buildEvenlyDistributedAgeTable(row_count) };
    std::vector<ColumnBatch> batches{ buildColumnBatches(table) };

    FilterOperation<std::greater<>> filter_op{ std::greater{}, compare_value };
    ColumnView target_column{ table.age.data() };
    Filter filter{};
    for (auto _ : state) {
        std::size_t total_selected{};

        for (const auto& batch : batches) {
            SelectedBatch selected_batch{ filter.apply(batch, target_column, filter_op) };
            total_selected += selected_batch.size;
            benchmark::DoNotOptimize(selected_batch.selection.data());
            benchmark::DoNotOptimize(selected_batch.size);
        }

        benchmark::DoNotOptimize(total_selected);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * row_count);
    state.SetBytesProcessed(state.iterations() * row_count * sizeof(std::uint64_t));
}

BENCHMARK(BM_FilterAge)
    ->Args({ 1024, 1 })
    ->Args({ 1024, 50 })
    ->Args({ 1024, 99 })
    ->Args({ 1 << 16, 1 })
    ->Args({ 1 << 16, 50 })
    ->Args({ 1 << 16, 99 })
    ->Args({ 1 << 24, 1 })
    ->Args({ 1 << 24, 50 })
    ->Args({ 1 << 24, 99 })
    ->Unit(benchmark::kMicrosecond);
} // namespace veq::bench
