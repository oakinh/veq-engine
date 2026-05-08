#include <gtest/gtest.h>
#include <veq/storage/table/table.hpp>
#include <veq/execution/filter.hpp>
#include <veq/execution/scan.hpp>
#include <veq/execution/projection.hpp>
#include "fixtures/table_fixtures.hpp"
#include "util/run_pipeline.hpp"

class PipelineTest : public testing::Test {
protected:
    veq::Table t0_ { veq::test::makeTinyTable() };
    veq::Scan s0_ { t0_, 4};
    veq::Filter f0_ {};
    veq::Projection p0_ {};
    const veq::FilterOperation<std::greater<>> filter_op0_ { std::greater<>{}, 30 };
    veq::ColumnView filter_target_column0_ { t0_.age.data() };
    std::vector<veq::ColumnView> column_views_ {
        veq::ColumnView{ t0_.id.data() },
        veq::ColumnView{ t0_.age.data() },
        veq::ColumnView{ t0_.occupation_id.data() }
    };
};

TEST_F(PipelineTest, PerformsNoCopies) {
    auto verifyPointerEquality = [] (
            const std::span<const veq::ColumnView> source_columns,
            const std::span<const veq::ColumnView> result_columns
        ) {
            ASSERT_GT(source_columns.size(), 0);
            ASSERT_GT(result_columns.size(), 0);
            for (std::size_t i {}; i < source_columns.size() && i < result_columns.size(); ++i) {
                ASSERT_EQ(source_columns[i].data, result_columns[i].data);
        }
    };

    std::size_t i {};
    while (s0_.hasNextBatch()) {
        const veq::ColumnBatch batch { s0_.nextBatch() };
        verifyPointerEquality(column_views_, batch.columns);
        p0_.setTargetColumns(column_views_);
        const veq::SelectedBatch selected_batch { f0_.apply(batch, filter_target_column0_, filter_op0_) };
        verifyPointerEquality(column_views_, selected_batch.columns);
        const veq::ProjectedBatch projected_batch { p0_.apply(selected_batch)};
        verifyPointerEquality(column_views_, projected_batch.columns);
        ++i;
    }
    ASSERT_GT(i, 0);
}

TEST_F(PipelineTest, DeliversExpectedResultFromSmallBatchAndTinyTable) {
    // std::size_t i {};
    // while (s0_.hasNextBatch()) {
    //     const veq::ColumnBatch batch { s0_.nextBatch() };
    //
    //     const veq::SelectedBatch selected_batch { f0_.apply(batch, filter_target_column0_, filter_op0_) };
    //     p0_.setTargetColumns(column_views_);
    //     const veq::ProjectedBatch projected_batch { p0_.apply(selected_batch)};
    //
    //     ++i;
    // }
    // ASSERT_GT(i, 0);
    veq::test::PipelineRunOutput output { veq::test::runPipeline<std::greater<>>({
        .scan=s0_,
        .filter=f0_,
        .projection=p0_,
        .projection_columns=column_views_,
        .filter_target_column=filter_target_column0_,
        .filter_op = filter_op0_
        }) };
}