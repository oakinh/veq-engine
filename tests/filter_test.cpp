#include <algorithm>
#include <gtest/gtest.h>
#include <ranges>
#include <veq/execution/filter.hpp>
#include <veq/execution/scan.hpp>

#include "fixtures/table_fixtures.hpp"

class FilterTest : public testing::Test {
  protected:
    veq::Table t0_{ veq::test::makeTinyTable() };
    veq::Scan s0_{ t0_, 4 };
    veq::Scan s1_{ t0_, 5 };
    veq::Scan s2_{ t0_, 1 };
    veq::Filter f0_{};
};

TEST_F(FilterTest, FiltersDataWithNonContiguousSelection) {
    const veq::FilterOperation<std::greater<>> filter_op{ std::greater<>{}, 30 };

    { // Batch 0
        veq::SelectedBatch batch0{ f0_.apply(s0_.nextBatch(), veq::ColumnView{ t0_.age.data() },
                                             filter_op) };
        EXPECT_EQ(batch0.size, 1);
        EXPECT_TRUE(std::ranges::contains(batch0.selection, 3));
    }

    { // Batch 1
        veq::SelectedBatch batch1{ f0_.apply(s0_.nextBatch(), veq::ColumnView{ t0_.age.data() },
                                             filter_op) };
        EXPECT_EQ(batch1.size, 2);
        EXPECT_TRUE(std::ranges::contains(batch1.selection, 7));
        EXPECT_TRUE(std::ranges::contains(batch1.selection, 6));
    }

    { // Batch 2
        veq::SelectedBatch batch2{ f0_.apply(s0_.nextBatch(), veq::ColumnView{ t0_.age.data() },
                                             filter_op) };
        EXPECT_EQ(batch2.size, 0);
    }
    EXPECT_FALSE(s0_.hasNextBatch());
}

TEST_F(FilterTest, ReturnsFullSelectionWhenAllRowsMatchWithBatchSize5) {
    const veq::FilterOperation<std::greater<>> filter_op{ std::greater<>{}, 0 };
    int i{};
    for (; s1_.hasNextBatch(); ++i) {
        veq::SelectedBatch batch{ f0_.apply(s1_.nextBatch(), veq::ColumnView{ t0_.age.data() },
                                            filter_op) };
        EXPECT_EQ(batch.size, 5);
    }
    EXPECT_EQ(i, 2);
}

TEST_F(FilterTest, ReturnsFullSelectionWhenAllRowsMatchWithBatchSize1) {
    const veq::FilterOperation<std::greater<>> filter_op{ std::greater<>{}, 0 };
    int i{};
    for (; s2_.hasNextBatch(); ++i) {
        veq::SelectedBatch batch{ f0_.apply(s2_.nextBatch(), veq::ColumnView{ t0_.age.data() },
                                            filter_op) };
        EXPECT_EQ(batch.size, 1);
    }
    EXPECT_EQ(i, 10);
}