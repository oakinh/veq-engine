#include <veq/execution/filter.hpp>
#include <veq/execution/scan.hpp>
#include <gtest/gtest.h>
#include "fixtures/table_fixtures.hpp"

class FilterTest : public testing::Test {
protected:
    veq::Table t0_ { veq::test::makeTinyTable() };
    veq::Scan s0_ { t0_, 4 };
};

TEST_F(FilterTest, FiltersDataAsExpected) {
    veq::Filter filter [[maybe_unused]] { t0_ };
}