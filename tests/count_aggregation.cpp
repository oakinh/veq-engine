#include <gtest/gtest.h>

#include <veq/execution/count_aggregation.hpp>
#include <veq/storage/table/table.hpp>

#include "fixtures/table_fixtures.hpp"

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
