#include <veq/execution/filter.hpp>
#include <veq/execution/scan.hpp>
#include <gtest/gtest.h>
#include <algorithm>
#include "fixtures/table_fixtures.hpp"

class FilterTest : public testing::Test {
protected:
    veq::Table t0_ { veq::test::makeTinyTable() };
    veq::Scan s0_ { t0_, 4 };
    veq::Filter f0_ {};
};

TEST_F(FilterTest, FiltersDataAsExpected) {
     const veq::FilterOperation<std::greater<>> filter_op { std::greater<>{}, 30 };

    { // Batch 0
        veq::SelectedBatch batch0 { f0_.apply(s0_.nextBatch(), veq::ColumnView{ t0_.age.data() }, filter_op) };
        EXPECT_EQ(batch0.selection.size(), 1);
        EXPECT_TRUE(std::ranges::contains(batch0.selection, 3));
    }

    { // Batch 1
        veq::SelectedBatch batch1 { f0_.apply(s0_.nextBatch(), veq::ColumnView{ t0_.age.data() }, filter_op) };
        EXPECT_EQ(batch1.selection.size(), 2);
        EXPECT_TRUE(std::ranges::contains(batch1.selection, 7));
        EXPECT_TRUE(std::ranges::contains(batch1.selection, 6));
    }

    { // Batch 2
        veq::SelectedBatch batch2 { f0_.apply(s0_.nextBatch(), veq::ColumnView{ t0_.age.data() }, filter_op) };
        EXPECT_EQ(batch2.selection.size(), 0);
    }
    EXPECT_FALSE(s0_.hasNextBatch());
}