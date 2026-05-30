#include <gtest/gtest.h>

#include <veq/execution/count_aggregation.hpp>
#include <veq/execution/scan.hpp>
#include <veq/storage/table/table.hpp>

#include "fixtures/table_fixtures.hpp"

class CountAggregationTest : public testing::Test {
protected:
    veq::Table t0_ { veq::test::makeTinyTable() };

    std::vector<veq::ColumnView> column_views_{ veq::ColumnView{ t0_.id.data() },
                                            veq::ColumnView{ t0_.age.data() },
                                            veq::ColumnView{ t0_.occupation_id.data() } };

    std::vector<std::size_t> selected_indexes { 0, 1, 2, 3, 5, 6, 7 };

    veq::SelectedBatch sb0_ {
        .columns = column_views_,
        .selection = selected_indexes,
        .size = selected_indexes.size()
    };
    veq::Scan scan_ { t0_ };
    veq::ColumnBatch cb0_ { scan_.nextBatch() };
};

// TEST(CountHashTableTest, HappyPath) {
//     veq::Table tiny_table { veq::test::makeTinyTable() };
//     veq::CountHashTable hash_table {};
//
//     for (std::size_t i {}; i < tiny_table.id.size(); ++i) {
//         hash_table.insert(tiny_table.occupation_id[i]);
//     }
//     const auto& buckets { hash_table.getBuckets() };
//
//     for (const auto& bucket : buckets) {
//         if (bucket.occupied_) {
//             ASSERT_GT(bucket.count_, 0);
//         }
//     }
//
// }
