#include <gtest/gtest.h>
#include <veq/execution/scan.hpp>
#include <veq/storage/table.hpp>
#include <veq/storage/batch.hpp>

class ScanTest : public testing::Test {
protected:
    ScanTest()
        : t0_ {
            .id = {
                1, 2, 3, 4, 5,
                6, 7, 8, 9, 10
            },
            .age = {
                25, 30, 25, 40, 30,
                25, 35, 40, 25, 30
            },
            .occupation_id = {
                100, 200, 100, 300, 200,
                100, 400, 300, 100, 200
            }
        },
          s0_ { t0_, 4 }
    {}
    veq::Table t0_;
    veq::Scan s0_;
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