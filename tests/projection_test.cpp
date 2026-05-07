#include <gtest/gtest.h>
#include <veq/execution/projection.hpp>
#include "fixtures/table_fixtures.hpp"

class ProjectionTest : public testing::Test {
protected:
    veq::Table t0_ { veq::test::makeTinyTable() };

    std::vector<veq::ColumnView> column_views {
        veq::ColumnView{ t0_.id.data() },
        veq::ColumnView{ t0_.age.data() },
        veq::ColumnView{ t0_.occupation_id.data() }
    };
    std::vector<std::size_t> selected_indexes { 3, 6, 7 };

    veq::SelectedBatch sb0_ {
        .columns = column_views,
        .selection = selected_indexes,
        .size = 3
    };

    veq::Projection p0_ {};
};

TEST_F(ProjectionTest, ProjectionPerformsZeroCopies) {
    p0_.setTargetColumns({ veq::ColumnView{ t0_.id.data() }});
    veq::ProjectedBatch projected_batch { p0_.apply(sb0_) };

    EXPECT_EQ(projected_batch.columns[0].data, t0_.id.data());
    EXPECT_EQ(projected_batch.selection_size, sb0_.size);
    EXPECT_EQ(projected_batch.selection.data(), sb0_.selection.data());
}

TEST_F(ProjectionTest, ProjectionReturnsOnlySelectedColumns) {
    p0_.setTargetColumns({ veq::ColumnView{ t0_.id.data() }});
    auto [ columns, selection, selection_size ] { p0_.apply(sb0_) };

    EXPECT_EQ(columns.size(), 1);
    EXPECT_EQ(columns[0].data, t0_.id.data());
}