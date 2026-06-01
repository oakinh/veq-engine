#pragma once
#include "../shared/util.hpp"
#include <random>
#include <cassert>
#include <veq/storage/table/table.hpp>

namespace veq::bench {
inline Table buildRandomAgeTable(std::size_t size) {
    Table table{};
    for (std::size_t i{}; i < size; ++i) {
        table.id.emplace_back(i);
        table.age.emplace_back(shared::generateRandomNumber(12345, 1, 100));
        table.occupation_id.emplace_back(i % 10);
    }
    return table;
}

inline Table buildEvenlyDistributedAgeTable(std::size_t size) {
    Table table{};
    for (std::size_t i{}; i < size; ++i) {
        table.id.emplace_back(i);
        table.age.emplace_back((i % 100) + 1);
        table.occupation_id.emplace_back(i % 10);
    }
    return table;
}

inline Table buildSequentialTable(std::size_t size) {
    Table table{};
    for (std::size_t i{}; i < size; ++i) {
        table.id.emplace_back(i);
        table.age.emplace_back(i);
        table.occupation_id.emplace_back(i);
    }
    return table;
}

inline Table buildVariableCardinalityOccupationTable(std::size_t size, std::size_t cardinality) {
    assert(cardinality > 0);
    Table table {};

    table.id.reserve(size);
    table.age.reserve(size);
    table.occupation_id.reserve(size);

    for (std::size_t i {}; i < size; ++i) {
        table.id.emplace_back(static_cast<std::uint64_t>(i));
        table.age.emplace_back(static_cast<std::uint64_t>(i));
        table.occupation_id.emplace_back(static_cast<std::uint64_t>(i % cardinality));
    }

    return table;
}
} // namespace veq::bench
