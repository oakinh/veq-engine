#pragma once
#include "test_materializer.hpp"
#include <cstdint>
#include <vector>
#include <veq/execution/filter.hpp>
#include <veq/execution/projection.hpp>
#include <veq/execution/scan.hpp>
#include <veq/execution/count_aggregation.hpp>
#include <veq/storage/column.hpp>
#include <veq/storage/table/schema.hpp>

namespace veq::test {
// TODO: Later these inputs need to be thought through more...
// We're sort of copying a ton of references to create a struct
// Seems quite wonky. It could be that my TS experience is giving me the wrong instincts here


template <typename Compare>
struct ScanFilterProjectRunInput {
    Scan& scan;
    Filter& filter;
    Projection& projection;
    const ColumnView& filter_target_column;
    const FilterOperation<Compare>& filter_op;
};

template<typename Compare>
struct RunPipelineAndMaterializeInput {
    Scan& scan;
    Filter& filter;
    Projection& projection;
    const ColumnViews& projection_columns;
    const ColumnView& filter_target_column;
    const FilterOperation<Compare>& filter_op;
    TestMaterializer& materializer_out;
};

template <typename Compare>
struct RunCountAggregationPipelineInput {
    Scan& scan;
    Filter& filter;
    CountAggregation& aggregation;
    const ColumnView& filter_target_column;
    const FilterOperation<Compare>& filter_op;
};

struct PipelineRunOutput {
    std::size_t batches_run{};
};

template <typename Compare>
ProjectedBatch scanFilterProject(ScanFilterProjectRunInput<Compare> inputs) {
    auto& [scan, filter, projection, filter_target_column, filter_op] = inputs;

    const ColumnBatch batch{ scan.nextBatch() };
    const SelectedBatch selected_batch{ filter.apply(batch, filter_target_column, filter_op) };

    return { projection.apply(selected_batch) };
}

template<typename Compare>
PipelineRunOutput runPipeline(RunPipelineAndMaterializeInput<Compare> inputs) {
    auto& [scan, filter, projection, projection_columns, filter_target_column, filter_op,
          materializer_out] = inputs;

    projection.setTargetColumns(projection_columns);

    std::size_t i{};
    while (scan.hasNextBatch()) {
        ProjectedBatch projected_batch { scanFilterProject<std::greater<>>({
            scan,
            filter,
            projection,
            filter_target_column,
            filter_op
            })};

        materializer_out.consume(projected_batch);
        ++i;
    }

    return { .batches_run = i };
}

template <typename Compare>
PipelineRunOutput runCountAggregationPipeline(RunCountAggregationPipelineInput<Compare> inputs) {
    auto& [scan, filter, aggregation, filter_target_column, filter_op] = inputs;

    std::size_t i{};
    while (scan.hasNextBatch()) {
        const ColumnBatch batch{ scan.nextBatch() };
        const SelectedBatch selected_batch{ filter.apply(batch, filter_target_column, filter_op) };

        aggregation.consume(selected_batch, static_cast<std::size_t>(ColumnName::OCCUPATION_ID));
        ++i;
    }

    aggregation.finalize();

    return { .batches_run = i };
}
} // namespace veq::test
