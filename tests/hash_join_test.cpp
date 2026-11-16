#include <gtest/gtest.h>
#include <cassert>
#include <numeric>
#include <algorithm>
#include <veq/execution/hash_join.hpp>
#include <veq/execution/projection.hpp>
#include <veq/storage/batch.hpp>
#include <veq/storage/table/table.hpp>

#include "fixtures/table_fixtures.hpp"

class HashJoinTest : public testing::Test {
protected:
    veq::Table t1_{ veq::test::makeTinyTable(1) };
    veq::Table t2_{ veq::test::makeTinyTable(2) };

    std::vector<veq::ColumnView> column_views_tiny_1_{ veq::ColumnView{ t1_.id.data() },
                                                veq::ColumnView{ t1_.age.data() },
                                                veq::ColumnView{ t1_.occupation_id.data() } };

    std::vector<veq::ColumnView> column_views_tiny_2_{ veq::ColumnView{ t2_.id.data() },
                                            veq::ColumnView{ t2_.age.data() },
                                            veq::ColumnView{ t2_.occupation_id.data() } };
    std::vector<std::size_t> selected_indexes_tiny_1_{ 0, 1, 2, 8 };
    std::vector<std::size_t> selected_indexes_tiny_2_{ 0, 1, 2, 7 };

    veq::ProjectedBatch pb1_{
        .columns = std::span(column_views_tiny_1_).first(2),
        .selection = selected_indexes_tiny_1_,
        .selection_size = 4 };
    veq::ProjectedBatch pb2_{
        .columns = std::span(column_views_tiny_2_).first(2),
        .selection = selected_indexes_tiny_2_,
        .selection_size = 4 };

    veq::HashJoin hash_join_1_ { pb1_.columns, pb2_.columns, 1, 1 };

    HashJoinTest() {
        for (std::size_t i : selected_indexes_tiny_1_) {
            assert(column_views_tiny_1_[1].data[i] == 40);
        }
        assert(column_views_tiny_2_[1].data[7] == 40);
    }

    void testHashJoinVariant(std::size_t build_size,
        std::size_t probe_size,
        std::size_t build_col_join_idx,
        std::size_t probe_col_join_idx,
        std::optional<std::vector<std::size_t>> build_selection = std::nullopt,
        std::optional<std::vector<std::size_t>> probe_selection = std::nullopt
        ) {
        const auto build { veq::test::buildEvenlyDistributedAgeAndOccupationTable(build_size) };
        const auto probe { veq::test::buildEvenlyDistributedAgeAndOccupationTable(probe_size) };
        auto build_cols { veq::test::getTableCols(build) };
        auto probe_cols { veq::test::getTableCols(probe) };

        veq::HashJoin hash_join { build_cols, probe_cols, build_col_join_idx, probe_col_join_idx };

        std::vector<std::size_t> build_indices {
            build_selection.has_value()
                ? build_selection.value()
                : std::views::iota(std::size_t{}, build_size)
                    | std::ranges::to<std::vector<std::size_t>>()
        };

        // Build phase
        hash_join.build(build_indices);

#ifndef NDEBUG
        ASSERT_EQ(build_indices.size(), hash_join.getHashTableRecordIdCount());
#endif

        std::vector<std::pair<std::size_t, std::size_t>> results {};

        // Probe phase
        std::vector<std::size_t> probe_indices {
            probe_selection.has_value()
                ? probe_selection.value()
                : std::views::iota(std::size_t{}, probe_size)
                    | std::ranges::to<std::vector<std::size_t>>()
        };

        std::vector<std::vector<std::size_t>> probe_batches {};

        std::vector<std::size_t> batch {};
        for (std::size_t idx : probe_indices) {
            if (batch.size() == veq::MAX_BATCH_SIZE) {
                probe_batches.push_back(batch);
                batch.clear();
            }
            batch.emplace_back(idx);
        }
        probe_batches.push_back(batch);

        std::size_t total_probe_count {};
        for (const auto& b : probe_batches) {
            total_probe_count += b.size();
        }

        std::size_t expected_probe_idx_count {
            probe_selection.has_value()
            ? probe_selection.value().size()
            : probe_size
        };

        ASSERT_EQ(expected_probe_idx_count, total_probe_count);

        for (const auto& probe_batch : probe_batches) {

            while (hash_join.probe(probe_batch)) {
                auto buffer { hash_join.getJoinBuffer() };
                EXPECT_EQ(buffer.size(), veq::MAX_BATCH_SIZE);
                results.append_range(buffer);
            }
            auto buffer { hash_join.getJoinBuffer() };
            ASSERT_LT(buffer.size(), veq::MAX_BATCH_SIZE);
            results.append_range(buffer);
        }


        // std::span remaining { probe_indices };
        //
        //  while (!remaining.empty()) {
        //      const auto count {
        //          std::min(veq::MAX_BATCH_SIZE, remaining.size())
        //      };
        //
        //      auto batch { remaining.first(count) };
        //
        //      while (hash_join.probe(batch)) {
        //          auto buffer { hash_join.getJoinBuffer() };
        //          EXPECT_EQ(buffer.size(), veq::MAX_BATCH_SIZE);
        //          results.append_range(buffer);
        //      }
        //
        //
        //      results.append_range(hash_join.getJoinBuffer());
        //
        //      remaining = remaining.subspan(count);
        //  }

        auto expected_results { veq::test::referenceInnerJoin(
            build_cols[build_col_join_idx],
            probe_cols[probe_col_join_idx],
            build_indices,
            probe_indices)
        };

        EXPECT_EQ(hash_join.total_joined_ids_, expected_results.size());
        std::sort(expected_results.begin(), expected_results.end());
        std::sort(results.begin(), results.end());
        ASSERT_GT(expected_results.size(), 0);
        EXPECT_EQ(expected_results.size(), results.size());
        for (std::size_t i {}; i < results.size(); ++i) {
            ASSERT_EQ(expected_results[i], results[i]);
        }
    }


};

TEST_F(HashJoinTest, OneToManyTinyJoin) {
    hash_join_1_.build(selected_indexes_tiny_1_);
    ASSERT_FALSE(hash_join_1_.probe(selected_indexes_tiny_2_));

    auto joined_ids { hash_join_1_.getJoinBuffer() };

    ASSERT_EQ(joined_ids.size(), 4);
    ASSERT_EQ(selected_indexes_tiny_1_.size(), 4);

    for (std::size_t i {}; i < 4; ++i) {
        ASSERT_EQ(joined_ids[i].first, selected_indexes_tiny_1_[i]);
        ASSERT_EQ(joined_ids[i].second, 7);
    }
}

TEST_F(HashJoinTest, BatchBoundariesHandledCorrectly) {
    auto build { veq::test::buildEvenlyDistributedAgeAndOccupationTable(2050) };
    auto probe { veq::test::buildEvenlyDistributedAgeAndOccupationTable(2050) };
    auto build_cols { veq::test::getTableCols(build) };
    auto probe_cols { veq::test::getTableCols(probe) };

    std::vector<std::vector<std::size_t>> selection_batches {
        {
            { 1023 },
            { 1024, 2047 },
            { 2048, 2049 },
        }};

    veq::HashJoin hash_join(build_cols, probe_cols, 0, 0);

    for (const auto& batch : selection_batches) {
        hash_join.build(batch);
    }

    std::vector<std::pair<veq::HashJoin::RecordId, veq::HashJoin::RecordId>> row_pair_results {};
    for (const auto& probe_batch : selection_batches) {
        while (hash_join.probe(probe_batch)) {
            row_pair_results.append_range(hash_join.getJoinBuffer());
        }
        row_pair_results.append_range(hash_join.getJoinBuffer());
    }


    std::vector<std::pair<veq::HashJoin::RecordId, veq::HashJoin::RecordId>> expected_results {
        {
            {1023, 1023},
            {1024, 1024},
            {2047, 2047},
            {2048, 2048},
            {2049, 2049}
        }};
    ASSERT_EQ(row_pair_results.size(), expected_results.size());

    for (std::size_t i {}; i < row_pair_results.size(); ++i) {
        EXPECT_EQ(row_pair_results[i], expected_results[i]);
    }
}

TEST_F(HashJoinTest, SparseSelectionAcrossMultipleBatches) {
    testHashJoinVariant(
        2050,
        3075,
        0,
        0,
        veq::test::selectModulo(2050, 2, 1),
        veq::test::selectModulo(3075, 3, 0)
        );
}

TEST_F(HashJoinTest, ManyToManyJoin) {
    testHashJoinVariant(
        2050,
        2050,
        1,
        1);
//     std::size_t build_size { 2050 };
//     std::size_t probe_size { 2050 };
//     std::size_t build_col_join_idx { 1 };
//     std::size_t probe_col_join_idx { 1 };
//     std::optional<std::vector<std::size_t>> build_selection { std::nullopt };
// std::optional<std::vector<std::size_t>> probe_selection { std::nullopt };
//
//     const auto build { veq::test::buildEvenlyDistributedAgeAndOccupationTable(build_size) };
//     const auto probe { veq::test::buildEvenlyDistributedAgeAndOccupationTable(probe_size) };
//     auto build_cols { veq::test::getTableCols(build) };
//     auto probe_cols { veq::test::getTableCols(probe) };
//
//     veq::HashJoin hash_join { build_cols, probe_cols, build_col_join_idx, probe_col_join_idx };
//
//     std::vector<std::size_t> build_indices {
//         build_selection.has_value()
//             ? build_selection.value()
//             : std::views::iota(std::size_t{}, build_size)
//                 | std::ranges::to<std::vector<std::size_t>>()
//     };
//
//     // Build phase
//     hash_join.build(build_indices);
//
// #ifndef NDEBUG
//     ASSERT_EQ(build_indices.size(), hash_join.getHashTableRecordIdCount());
// #endif
//
//     std::vector<std::pair<std::size_t, std::size_t>> results {};
//
//     // Probe phase
//     std::vector<std::size_t> probe_indices {
//         probe_selection.has_value()
//             ? probe_selection.value()
//             : std::views::iota(std::size_t{}, probe_size)
//                 | std::ranges::to<std::vector<std::size_t>>()
//     };
//     std::span remaining { probe_indices };
//
//     while (!remaining.empty()) {
//         const auto count {
//             std::min(veq::MAX_BATCH_SIZE, remaining.size())
//         };
//
//         auto batch { remaining.first(count) };
//
//         while (hash_join.probe(batch)) {
//             results.append_range(hash_join.getJoinBuffer());
//         }
//
//         results.append_range(hash_join.getJoinBuffer());
//
//         remaining = remaining.subspan(count);
//     }
//
//     auto expected_results { veq::test::referenceInnerJoin(
//         build_cols[build_col_join_idx],
//         probe_cols[probe_col_join_idx],
//         build_indices,
//         probe_indices)
//     };
//
//     ASSERT_GT(expected_results.size(), 0);
//     ASSERT_EQ(expected_results.size(), results.size());
//     for (std::size_t i {}; i < results.size(); ++i) {
//         EXPECT_EQ(expected_results[i], results[i]);
//     }
}



