#include <gtest/gtest.h>
#include <veq/execution/projection.hpp>
#include "fixtures/table_fixtures.hpp"

class ProjectionTest : public testing::Test {
protected:
    veq::Table t0_ { veq::test::makeTinyTable() };

    std::vector<veq::ColumnView> column_views_ {
        veq::ColumnView{ t0_.id.data() },
        veq::ColumnView{ t0_.age.data() },
        veq::ColumnView{ t0_.occupation_id.data() }
    };
    std::vector<std::size_t> selected_indexes_ { 3, 6, 7 };

    veq::SelectedBatch sb0_ {
        .columns = column_views_,
        .selection = selected_indexes_,
        .size = 3
    };

    veq::Projection p0_ {};
};

TEST_F(ProjectionTest, PerformsZeroCopies) {
    p0_.setTargetColumns({ veq::ColumnView{ t0_.id.data() }});
    veq::ProjectedBatch projected_batch { p0_.apply(sb0_) };

    EXPECT_EQ(projected_batch.columns[0].data, t0_.id.data());
    EXPECT_EQ(projected_batch.selection_size, sb0_.size);
    EXPECT_EQ(projected_batch.selection.data(), sb0_.selection.data());
}

TEST_F(ProjectionTest, ProjectionReturnsOnlySelectedColumn) {
    p0_.setTargetColumns({ veq::ColumnView{ t0_.id.data() }});
    auto [ columns, selection, selection_size ] { p0_.apply(sb0_) };

    EXPECT_EQ(columns.size(), 1);
    EXPECT_EQ(columns[0].data, t0_.id.data());
}

TEST_F(ProjectionTest, ReturnsMultipleSelectedColumns) {
    p0_.setTargetColumns({
        veq::ColumnView{ t0_.id.data() },
        veq::ColumnView{ t0_.age.data() },
        veq::ColumnView{ t0_.occupation_id.data() }
    });

    auto [ columns, selection, selection_size ] { p0_.apply(sb0_) };
    EXPECT_EQ(columns.size(), 3);
}

TEST_F(ProjectionTest, PreservesTargetColumnOrder) {
    const veq::ColumnViews target_columns {
        veq::ColumnView{ t0_.age.data() },
        veq::ColumnView{ t0_.id.data() },
        veq::ColumnView{ t0_.occupation_id.data() }
    };

    p0_.setTargetColumns(target_columns);

    auto [ columns, selection, selection_size ] { p0_.apply(sb0_) };
    for (std::size_t i {}; i < target_columns.size(); ++i) {
        ASSERT_EQ(target_columns[i].data, columns[i].data);
    }
}