#include <gtest/gtest.h>

#include <unordered_map>

#include <veq/execution/count_aggregation.hpp>
#include <veq/execution/scan.hpp>
#include <veq/storage/table/table.hpp>

#include "../cmake-build-debug/_deps/googletest-src/googletest/include/gtest/gtest.h"
#include "fixtures/table_fixtures.hpp"

class CountAggregationTest : public testing::Test {
protected:
    veq::Table t0_ { veq::test::makeTinyTable() };

    std::vector<veq::ColumnView> column_views_{ veq::ColumnView{ t0_.id.data() },
                                            veq::ColumnView{ t0_.age.data() },
                                            veq::ColumnView{ t0_.occupation_id.data() } };

    std::vector<std::size_t> selected_indexes { 0, 1, 2, 3, 5, 6, 7 };

    veq::SelectedBatch sb0_ {
        .columns = column_views_,
        .selection = selected_indexes,
        .size = selected_indexes.size()
    };

    veq::Scan scan_ { t0_ };
    veq::ColumnBatch cb0_ { scan_.nextBatch() };

    veq::CountAggregation aggregation {};
};

TEST_F(CountAggregationTest, CorrectlySumsFromSelectedBatch) {
    aggregation.consume(sb0_, 2);
    aggregation.finalize();
    const veq::CountAggregationResult& result { aggregation.result() };

    ASSERT_EQ(result.keys_.size(), result.counts_.size());

    std::unordered_map<std::uint64_t, std::uint64_t> expected_output = {
        { 100, 3 },
        { 200, 1 },
        { 300, 2 },
        { 400, 1 }
    };

    ASSERT_EQ(expected_output.size(), result.keys_.size());

    for (std::size_t i {}; i < result.keys_.size(); ++i) {

        const auto& expected_count { expected_output.find(result.keys_[i]) };
        if (expected_count == expected_output.end()) GTEST_FAIL();

        ASSERT_EQ(expected_count->second, result.counts_[i]);
    }
}

TEST_F(CountAggregationTest, CorrectlySumsFromColumnBatch) {
    aggregation.consume(cb0_, 2);
    aggregation.finalize();
    const veq::CountAggregationResult& result { aggregation.result() };

    ASSERT_EQ(result.keys_.size(), result.counts_.size());

    std::unordered_map<std::uint64_t, std::uint64_t> expected_output = {
        { 100, 4 },
        { 200, 3 },
        { 300, 2 },
        { 400, 1 }
    };

    ASSERT_EQ(expected_output.size(), result.keys_.size());

    for (std::size_t i {}; i < result.keys_.size(); ++i) {

        const auto& expected_count { expected_output.find(result.keys_[i]) };
        if (expected_count == expected_output.end()) GTEST_FAIL();

        ASSERT_EQ(expected_count->second, result.counts_[i]);
    }
}

// TEST(CountHashTableTest, HappyPath) {
//     veq::Table tiny_table { veq::test::makeTinyTable() };
//     veq::CountHashTable hash_table {};
//
//     for (std::size_t i {}; i < tiny_table.id.size(); ++i) {
//         hash_table.insert(tiny_table.occupation_id[i]);
//     }
//     const auto& buckets { hash_table.getBuckets() };
//
//     for (const auto& bucket : buckets) {
//         if (bucket.occupied_) {
//             ASSERT_GT(bucket.count_, 0);
//         }
//     }
//
// }
