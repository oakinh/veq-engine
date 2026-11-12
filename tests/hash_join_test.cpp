#include <gtest/gtest.h>
#include <cassert>
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

    veq::HashJoin hash_join_ { pb1_.columns, pb2_.columns, 1, 1 };

    HashJoinTest() {
        for (std::size_t i : selected_indexes_tiny_1_) {
            assert(column_views_tiny_1_[1].data[i] == 40);
        }
        assert(column_views_tiny_2_[1].data[7] == 40);
    }
};

TEST_F(HashJoinTest, OneToManyTinyJoin) {
    hash_join_.build(selected_indexes_tiny_1_);
    ASSERT_FALSE(hash_join_.probe(selected_indexes_tiny_2_));

    auto joined_ids { hash_join_.getJoinBuffer() };

    ASSERT_EQ(joined_ids.size(), 4);
    ASSERT_EQ(selected_indexes_tiny_1_.size(), 4);

    for (std::size_t i {}; i < 4; ++i) {
        ASSERT_EQ(joined_ids[i].first, selected_indexes_tiny_1_[i]);
        ASSERT_EQ(joined_ids[i].second, 7);
    }
}


