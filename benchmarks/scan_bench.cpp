#include <benchmark/benchmark.h>

#include "util/table_builders.hpp"
#include <cstddef>
#include <veq/execution/scan.hpp>

namespace veq::bench {
constexpr std::size_t batch_size = 1024;

static void BM_ScanColumns(benchmark::State& state) {
    auto row_count{ state.range(0) };
    Table table{ buildSequentialTable(row_count) };

    for (auto _ : state) {
        Scan scan{ table };

        std::uint64_t sum{};

        while (scan.hasNextBatch()) {
            auto batch{ scan.nextBatch() };

            auto id_col{ batch.columns[0] };

            for (std::size_t i{ batch.start_row }; i < batch.start_row + batch.size; ++i) {
                sum += id_col.data[i];
            }
        }
        benchmark::DoNotOptimize(sum);
    }

    state.SetItemsProcessed(state.iterations() * row_count);
    state.SetBytesProcessed(state.iterations() * row_count * sizeof(std::uint64_t));
}

BENCHMARK(BM_ScanColumns)
    ->RangeMultiplier(2)
    ->Range(batch_size, 1 << 24)
    ->Unit(benchmark::kMicrosecond);
} // namespace veq::bench
