#include <algorithm>
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

class CountHashTableTest : public testing::Test {
protected:
    veq::CountHashTable ht0_ {};
};

// CountAggregation Tests

TEST_F(CountAggregationTest, FinalizeCalledBeforeConsumeMaterializesSizeZeroResult) {
    aggregation.finalize();
    const veq::CountAggregationResult& result { aggregation.result() };

    ASSERT_EQ(result.keys_.size(), 0);
    ASSERT_EQ(result.counts_.size(), 0);
}

TEST_F(CountAggregationTest, NoDuplicateKeys) {
    aggregation.consume(sb0_, 2);
    aggregation.consume(cb0_, 2);
    aggregation.finalize();

    std::unordered_map<std::uint64_t, std::uint64_t> seen_key_tracker {};

    const veq::CountAggregationResult& result { aggregation.result() };

    for (std::size_t i {}; i < result.keys_.size(); ++i) {
        auto key { result.keys_[i] };
        ASSERT_EQ(seen_key_tracker.find(key), seen_key_tracker.end());
        seen_key_tracker.emplace(key, result.counts_[i]);
    }
}

TEST_F(CountAggregationTest, CorrectConsumeUsageDoesntThrow) {
    EXPECT_NO_THROW(aggregation.consume(sb0_, 2));
}

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

// CountHashTable Tests

TEST_F(CountHashTableTest, ResetSetsOccupiedCountToZeroAndBoolsToFalse) {
    for (std::size_t i {}; i < 50; ++i) {
        ht0_.insert(i);
    }

    ht0_.reset();

    ASSERT_EQ(ht0_.getOccupiedCount(), 0);

    const auto& buckets { ht0_.getBuckets() };
    for (const auto& bucket : buckets) {
        ASSERT_FALSE(bucket.occupied_);
    }
}

TEST_F(CountHashTableTest, RehashPreservesElements) {
    for (std::size_t i { 1 }; i <= 1000; ++i) {
        ht0_.insert(i);
    }

    const auto& buckets { ht0_.getBuckets() };

    std::unordered_map<uint64_t, uint64_t> unique_tracker {};
    std::size_t occupied_count {};
    std::size_t unoccupied_count {};
    for (const auto& bucket : buckets) {
        if (bucket.occupied_) {
            ASSERT_GE(bucket.key_, 1);
            ASSERT_LE(bucket.key_, 1000);
            ASSERT_EQ(bucket.count_, 1);
            unique_tracker.emplace(bucket.key_, 1);
            ++occupied_count;
        } else {
            ++unoccupied_count;
        }
    }
    ASSERT_EQ(occupied_count, ht0_.getOccupiedCount());
    ASSERT_EQ(unique_tracker.size(), 1000);
    ASSERT_TRUE(static_cast<double>(1000) / static_cast<double>(1000 + unoccupied_count) <= 0.70);
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
