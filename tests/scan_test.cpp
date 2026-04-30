#include <gtest/gtest.h>
#include <veq/execution/scan.hpp>
#include <veq/storage/table.hpp>
#include <veq/storage/batch.hpp>
#include <fixtures/table_fixtures.hpp>

class ScanTest : public testing::Test {
protected:
    veq::Table t0_ { veq::test::makeTinyTable() };
    veq::Scan s0_ { t0_, 4 };
};

TEST_F(ScanTest, NextBatchIteratesThroughColumns) {
    veq::ColumnBatch batch0 { s0_.nextBatch() };
    EXPECT_EQ(batch0.size, 4);
    EXPECT_EQ(batch0.start_row, 0);
    EXPECT_EQ(batch0.columns.size(), 3);

    veq::ColumnBatch batch1 { s0_.nextBatch() };
    EXPECT_EQ(batch1.size, 4);
    EXPECT_EQ(batch1.start_row, 4);
    EXPECT_EQ(batch1.columns.size(), 3);

    veq::ColumnBatch batch2 { s0_.nextBatch() };
    EXPECT_EQ(batch2.size, 2);
    EXPECT_EQ(batch2.start_row, 8);
    EXPECT_EQ(batch2.columns.size(), 3);
}