#pragma once
#include <vector>
#include <cstdint>
#include <veq/storage/column.hpp>
#include <veq/execution/scan.hpp>
#include <veq/execution/filter.hpp>
#include <veq/execution/projection.hpp>
#include "test_materializer.hpp"

namespace veq::test {
    template <typename Compare>
    struct PipelineRunInput {
        Scan& scan;
        Filter& filter;
        Projection& projection;
        std::span<ColumnView>projection_columns;
        ColumnView filter_target_column;
        FilterOperation<Compare> filter_op;
    };

    struct PipelineRunOutput {
        TestMaterializer materializer {};
        std::size_t batches_run {};
    };

    template <typename Compare>
    PipelineRunOutput runPipeline(PipelineRunInput<Compare> inputs) {
        auto [scan,
            filter,
            projection,
            projection_columns,
            filter_target_column,
            filter_op
            ] = inputs;
        TestMaterializer test_materializer {};

        std::size_t i {};
        while (scan.hasNextBatch()) {
            const ColumnBatch batch { scan.nextBatch() };

            projection.setTargetColumns(projection_columns);
            const SelectedBatch selected_batch { filter.apply(
                batch,
                filter_target_column,
                filter_op)
            };

            const ProjectedBatch projected_batch { projection.apply(selected_batch)};
            test_materializer.consume(projected_batch);
            ++i;
        }

        return {
            .materializer = test_materializer,
            .batches_run = i
        };
    }
}
