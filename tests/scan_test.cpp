#include <gtest/gtest.h>
#include <veq/execution/scan.hpp>
#include <veq/storage/table.hpp>
#include <veq/storage/batch.hpp>
#include <fixtures/table_fixtures.hpp>

class ScanTest : public testing::Test {
protected:
    veq::Table t0_ { veq::test::makeTinyTable() };
    veq::Table t1_ {}; // Empty table
    veq::Table t2_ { veq::test::makeSizeOneTable() };
    veq::Scan s0_ { t0_, 4 };
    veq::Scan s1_ { t0_, 1 };

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

TEST_F(ScanTest, ReturnsBatchOfOneWithSizeOneBatch) {
    std::size_t i {};
    for (; s1_.hasNextBatch(); ++i) {
        veq::ColumnBatch batch { s1_.nextBatch() };
        EXPECT_EQ(batch.size, 1);
        EXPECT_EQ(t0_.id[i], batch.columns[0].data[i]);
        EXPECT_EQ(batch.start_row, i);
    }
}

TEST_F(ScanTest, ReturnsBatchWithEntireTableWhenBatchSizeIsGreaterThanTableSize) {
    veq::Scan scan { t0_, 11 };
    veq::ColumnBatch batch { scan.nextBatch() };
    EXPECT_EQ(batch.size, t0_.id.size());
    // for (std::size_t i {}; i < t0_.id.size(); ++i) {
    //     batch.
    // }
}