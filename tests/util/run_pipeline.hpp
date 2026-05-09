#pragma once
#include "test_materializer.hpp"
#include <cstdint>
#include <vector>
#include <veq/execution/filter.hpp>
#include <veq/execution/projection.hpp>
#include <veq/execution/scan.hpp>
#include <veq/storage/column.hpp>

namespace veq::test {
template<typename Compare> struct PipelineRunInput {
    Scan& scan;
    Filter& filter;
    Projection& projection;
    const ColumnViews& projection_columns;
    const ColumnView& filter_target_column;
    const FilterOperation<Compare>& filter_op;
    TestMaterializer& materializer_out;
};

struct PipelineRunOutput {
    std::size_t batches_run{};
};

template<typename Compare> PipelineRunOutput runPipeline(PipelineRunInput<Compare> inputs) {
    auto [scan, filter, projection, projection_columns, filter_target_column, filter_op,
          materializer_out] = inputs;

    std::size_t i{};
    while (scan.hasNextBatch()) {
        const ColumnBatch batch{scan.nextBatch()};

        projection.setTargetColumns(projection_columns);
        const SelectedBatch selected_batch{filter.apply(batch, filter_target_column, filter_op)};

        const ProjectedBatch projected_batch{projection.apply(selected_batch)};
        materializer_out.consume(projected_batch);
        ++i;
    }

    return {.batches_run = i};
}
} // namespace veq::test
