#include "fixtures/table_fixtures.hpp"
#include "util/run_pipeline.hpp"
#include <gtest/gtest.h>
#include <veq/execution/filter.hpp>
#include <veq/execution/projection.hpp>
#include <veq/execution/scan.hpp>
#include <veq/storage/table/table.hpp>

class PipelineTest : public testing::Test {
  protected:
    veq::Table t0_{ veq::test::makeTinyTable() };
    veq::Table t1_{}; // Empty table
    veq::Scan s0_{ t0_, 4 };
    veq::Scan s1_{ t1_, 4 }; // Scan on empty table
    veq::Filter f0_{};
    veq::Projection p0_{};
    veq::CountAggregation ca0_ {};
    const veq::FilterOperation<std::greater<>> filter_op0_{ std::greater<>{}, 30 };
    veq::ColumnView filter_target_column0_{ t0_.age.data() };
    std::vector<veq::ColumnView> column_views_{ veq::ColumnView{ t0_.id.data() },
                                                veq::ColumnView{ t0_.age.data() },
                                                veq::ColumnView{ t0_.occupation_id.data() } };
};

// These are for testing the entire pipeline of operators together, not just in isolation

TEST_F(PipelineTest, PerformsNoCopies) {
    auto verifyPointerEquality = [](const std::span<const veq::ColumnView> source_columns,
                                    const std::span<const veq::ColumnView> result_columns) {
        ASSERT_GT(source_columns.size(), 0);
        ASSERT_GT(result_columns.size(), 0);
        for (std::size_t i{}; i < source_columns.size() && i < result_columns.size(); ++i) {
            ASSERT_EQ(source_columns[i].data, result_columns[i].data);
        }
    };

    std::size_t i{};
    while (s0_.hasNextBatch()) {
        const veq::ColumnBatch batch{ s0_.nextBatch() };
        verifyPointerEquality(column_views_, batch.columns);
        p0_.setTargetColumns(column_views_);
        const veq::SelectedBatch selected_batch{ f0_.apply(batch, filter_target_column0_,
                                                           filter_op0_) };
        verifyPointerEquality(column_views_, selected_batch.columns);
        const veq::ProjectedBatch projected_batch{ p0_.apply(selected_batch) };
        verifyPointerEquality(column_views_, projected_batch.columns);
        ++i;
    }
    ASSERT_GT(i, 0);
}

TEST_F(PipelineTest, DeliversExpectedResultFromSmallBatchAndTinyTable) {
    veq::test::TestMaterializer materializer{};

    // TODO: Figure out if there's a way around having to pass template types for compare since it's already part of the struct
    veq::test::PipelineRunOutput output{ veq::test::runPipeline<std::greater<>>(
        { .scan = s0_,
          .filter = f0_,
          .projection = p0_,
          .projection_columns = column_views_,
          .filter_target_column = filter_target_column0_,
          .filter_op = filter_op0_,
          .materializer_out = materializer }) };
    ASSERT_GT(output.batches_run, 0);
    std::vector<std::vector<uint64_t>> columns{ materializer.columns() };
    ASSERT_EQ(columns.size(), 3);

    std::vector<std::vector<uint64_t>> expected_output{
        { 4, 7, 8 },      // id
        { 40, 35, 40 },   // age
        { 300, 400, 300 } // occupation_id
    };
    std::size_t i{};
    for (std::size_t col_idx{}; col_idx < columns.size(); ++col_idx) {
        ++i;
        const auto& result_column{ columns[col_idx] };
        const auto& expected_column{ expected_output[col_idx] };
        for (std::size_t row_idx{}; row_idx < columns[col_idx].size(); ++row_idx) {
            const auto& result_val{ result_column[row_idx] };
            const auto& expected_val{ expected_column[row_idx] };
            EXPECT_EQ(result_val, expected_val);
        }
    }
    ASSERT_GT(i, 0);
}

TEST_F(PipelineTest, EmptyTableDoesntCrash) {
    veq::test::TestMaterializer materializer{};
    veq::test::PipelineRunOutput output{ veq::test::runPipeline<std::greater<>>(
        { .scan = s1_,
          .filter = f0_,
          .projection = p0_,
          .projection_columns = column_views_,
          .filter_target_column = filter_target_column0_,
          .filter_op = filter_op0_,
          .materializer_out = materializer }) };
    ASSERT_EQ(output.batches_run, 0);
    std::vector columns{ materializer.columns() };
    ASSERT_EQ(columns.size(), 0);
}

TEST_F(PipelineTest, CountAggregationRunYieldsExpectedResult) {
    std::unordered_map<std::uint64_t, std::uint64_t> expected_count_result {
        { 300, 2 },
        { 400, 1 },
    };
    veq::test::PipelineRunOutput output { veq::test::runCountAggregationPipeline<std::greater<>>({
        .scan = s0_,
        .filter = f0_,
        .aggregation = ca0_,
        .filter_target_column = filter_target_column0_,
        .filter_op = filter_op0_,
    })};

    ASSERT_GT(output.batches_run, 0);

    const auto& result { ca0_.result() };

    ASSERT_GT(result.keys_.size(), 0); // Make sure it actually ran
    ASSERT_EQ(expected_count_result.size(), result.keys_.size()); // Invariant
    for (std::size_t i {}; i < result.keys_.size(); ++i) {
        auto key { result.keys_[i] };
        auto count { result.counts_[i] };
        auto it { expected_count_result.find(key) };
        if (it == expected_count_result.end()) GTEST_FAIL();

        ASSERT_EQ(it->second, count);
    }
}