#include <gtest/gtest.h>
#include <veq/execution/projection.hpp>
#include "fixtures/table_fixtures.hpp"

/*
struct SelectedBatch {
    std::span<const ColumnView> columns {};
    std::span<const std::size_t> selection {}; // Global indexes
    std::size_t size {};
}
*/

class ProjectionTest : public testing::Test {
protected:
    veq::Table t0_ { veq::test::makeTinyTable() };

    veq::SelectedBatch sb0_ {
        .columns = column_views,
        .size = 4,
        .selection = selected_indexes
    };

    veq::Projection p0_ {};

private:
    std::vector<veq::ColumnView> column_views {
        veq::ColumnView{ t0_.id.data() },
        veq::ColumnView{ t0_.age.data() },
        veq::ColumnView{ t0_.occupation_id.data() }
    };
    std::vector<std::size_t> selected_indexes { 3, 6, 7 };
};

TEST_F(ProjectionTest, ProjectionPerformsZeroCopies) {
    veq::ProjectedBatch projected_batch { p0_.apply(sb0_) };
    p0_.setTargetColumns({ veq::ColumnView{ t0_.id.data() }});

    EXPECT_EQ(projected_batch.columns[0].data, t0_.id.data());
    EXPECT_EQ(projected_batch.size, sb0_.size);
    EXPECT_EQ(projected_batch.selection.data(), sb0_.selection.data());

}