#include <benchmark/benchmark.h>
#include <cstddef>
#include <veq/execution/filter.hpp>
#include "util/column_batch_builders.hpp"

namespace veq::bench {
    static void BM_FilterAge(benchmark::State& state) {
        auto row_count { state.range(0) };
        Table table { buildRandomAgeTable(row_count) };
        std::vector<ColumnBatch> batches { buildBatches(table) };

        Filter filter {};
        FilterOperation<std::greater<>> filter_op { std::greater{}, 50 };

        for (auto _ : state) {

            for (const auto& batch : batches) {
                filter.apply(batch, ColumnView{ table.age.data() }, filter_op);
            }

        }
    }
}

