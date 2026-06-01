#pragma once
#include <cassert>
#include <veq/storage/table/table.hpp>


namespace veq::test {
inline Table makeTinyTable() {
    // Size 10
    Table table { .id = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 },
                  .age = { 25, 30, 25, 40, 30, 25, 35, 40, 25, 30 },
                  .occupation_id = { 100, 200, 100, 300, 200, 100, 400, 300, 100, 200 } };
    assert(table.id.size() == table.age.size() && table.id.size() == table.occupation_id.size());
    return table;
}

inline Table makeSizeOneTable() {
    return Table{ .id = { 1 }, .age = { 12 }, .occupation_id = { 25 } };
}

inline Table buildEvenlyDistributedAgeAndOccupationTable(std::size_t size) {
    Table table{};
    for (std::size_t i{}; i < size; ++i) {
        table.id.emplace_back(i);
        table.age.emplace_back((i % 100) + 1);
        table.occupation_id.emplace_back(i % 10);
    }
    return table;
}
} // namespace veq::test