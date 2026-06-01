#include <benchmark/benchmark.h>

#include <veq/execution/count_aggregation.hpp>

#include "util/column_batch_builders.hpp"
#include "util/table_builders.hpp"

namespace veq::bench {
    static void BM_CountAggregationFixedCardinality(benchmark::State& state) {
        std::size_t row_count { static_cast<std::size_t>(state.range(0)) };
        std::size_t cardinality { static_cast<std::size_t>(state.range(1)) };

        Table table { buildVariableCardinalityOccupationTable(row_count, cardinality) };

        std::vector<ColumnBatch> batches { buildColumnBatches(table) };

        CountAggregation aggregation {};

        for (auto _ : state) {
            aggregation.reset();

            std::size_t rows_seen {};

            for (const auto& batch : batches) {
                rows_seen += batch.size;
                aggregation.consume(batch, static_cast<std::size_t>(ColumnName::OCCUPATION_ID));
            }
            aggregation.finalize();
            const auto& result { aggregation.result() };

            std::uint64_t total_count {};
            for (auto count : result.counts_) {
                total_count += count;
            }

            if (rows_seen != row_count) {
                state.SkipWithError("batches do not cover row_count");
                break;
            }

            if (total_count != row_count) {
                state.SkipWithError("aggregation counts do not sum to row_count");
                break;
            }

            benchmark::DoNotOptimize(total_count);
            benchmark::DoNotOptimize(result.keys_.data());
            benchmark::DoNotOptimize(result.counts_.data());
            benchmark::DoNotOptimize(result.keys_.size());
            benchmark::DoNotOptimize(result.counts_.size());
        }

        state.SetItemsProcessed(state.iterations() * row_count);
        state.SetBytesProcessed(state.iterations() * row_count * sizeof(std::uint64_t));
    }

    BENCHMARK(BM_CountAggregationFixedCardinality)
        ->Args({ 1024, 1 })
        ->Args({ 1024, 8 })
        ->Args({ 1024, 1024 })
        ->Args({ 1 << 16, 1 })
        ->Args({ 1 << 16, 8 })
        ->Args({ 1 << 16, 1024 })
        ->Args({ 1 << 16, 1 << 16 })
        ->Args({ 1 << 24, 1 })
        ->Args({ 1 << 24, 8 })
        ->Args({ 1 << 24, 1024 })
        ->Args({ 1 << 24, 1 << 16 })
        ->Args({ 1 << 24, 1 << 24 })
        ->Unit(benchmark::kMicrosecond);
}

