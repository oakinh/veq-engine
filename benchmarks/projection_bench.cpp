#include <benchmark/benchmark.h>
#include <veq/storage/batch.hpp>
#include <veq/execution/projection.hpp>

#include "util/column_batch_builders.hpp"
#include "util/selected_batch_builders.hpp"
#include "util/bench_materializer.hpp"

namespace veq::bench {
    static void BM_Projection(benchmark::State& state) {
        auto row_count { static_cast<std::size_t>(state.range(0)) };
        auto compare_value { static_cast<uint64_t>(state.range(1)) };

        // TODO: Build a table with more columns
        Table table { buildEvenlyDistributedAgeTable(row_count) };

        // OwnedSelectedBatch is a benchmark-only structure, because SelectedBatch's selection is a view...
        // ...into a std::vector reusable buffer held by the Filter operator
        std::vector<OwnedSelectedBatch> owned_selected_batches { buildOwnedSelectedBatches(
            buildColumnBatches(table),
            ColumnView{ table.age.data() },
            FilterOperation(std::greater{}, compare_value)
            )
        };

        std::vector<SelectedBatch> selected_batches { convertOwnedSelectedBatches(owned_selected_batches) };


        Projection projection {};
        projection.setTargetColumns({ ColumnView{table.id.data() } });

        BenchMaterializer materializer {};

        for (auto _ : state) {
            std::size_t total_selected {};
            materializer.reset();

            for (const auto& selected_batch : selected_batches) {
                ProjectedBatch pj { projection.apply(selected_batch) };
                total_selected += pj.selection_size;
                materializer.consume(pj);

                benchmark::DoNotOptimize(pj.selection.data());
                benchmark::DoNotOptimize(pj.columns.data());
                benchmark::DoNotOptimize(pj.selection_size);
            }

            const auto cols { materializer.columns() };

            benchmark::DoNotOptimize(total_selected);
        }

        state.SetItemsProcessed(state.iterations() * row_count);
        state.SetBytesProcessed(state.iterations() * row_count * sizeof(std::uint64_t));
    }
    BENCHMARK(BM_Projection)
        ->Args({1024, 1})
        ->Args({1024, 50})
        ->Args({1024, 99})
        ->Args({1 << 16, 1})
        ->Args({1 << 16, 50})
        ->Args({1 << 16, 99})
        ->Args({1 << 24, 1})
        ->Args({1 << 24, 50})
        ->Args({1 << 24, 99})
        ->Unit(benchmark::kMicrosecond);
}

