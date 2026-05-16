#include <benchmark/benchmark.h>
#include <veq/storage/batch.hpp>
#include <veq/execution/projection.hpp>

#include "util/column_batch_builders.hpp"
#include "util/selected_batch_builders.hpp"

namespace veq::bench {
    // static void BM_Projection(benchmark::State& state) {
    //     auto row_count { state.range(0) };
    //     Projection projection {};
    //
    //     Table table { buildRandomAgeTable(row_count) };
    //
    //     std::vector<SelectedBatch> selected_batches { buildSelectedBatches(
    //         buildColumnBatches(table),
    //         ColumnView{ table.age.data() },
    //         FilterOperation(std::greater{}, 50)
    //         )
    //     };
    //
    //     //std::vector<ProjectedBatch> projected_batches {};
    //     std::size_t total_selected {};
    //
    //     for (auto _ : state) {
    //
    //         for (const auto& selected_batch : selected_batches) {
    //             ProjectedBatch pj { projection.apply(selected_batch) };
    //             total_selected += pj.selection_size;
    //             benchmark::DoNotOptimize(pj.selection.data());
    //         }
    //
    //         benchmark::DoNotOptimize(total_selected);
    //     }
    //
    //     state.SetItemsProcessed(state.iterations() * row_count);
    //     state.SetBytesProcessed(state.iterations() * row_count * sizeof(std::uint64_t));
    // }
}

