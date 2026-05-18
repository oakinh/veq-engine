#pragma once
#include <veq/storage/table/table.hpp>
#include <random>
#include "../shared/util.hpp"

namespace veq::bench {
    inline Table buildRandomAgeTable(std::size_t size) {
        Table table {};
        for (std::size_t i {}; i < size; ++i) {
            table.id.emplace_back(i);
            table.age.emplace_back(shared::generateRandomNumber(12345, 1, 100));
            table.occupation_id.emplace_back(i % 10);
        }
        return table;
    }

    inline Table buildEvenlyDistributedAgeTable(std::size_t size) {
        Table table {};
        for (std::size_t i {}; i < size; ++i) {
            table.id.emplace_back(i);
            table.age.emplace_back((i % 100) + 1);
            table.occupation_id.emplace_back(i % 10);
        }
        return table;
    }

    inline Table buildSequentialTable(std::size_t size) {
        Table table {};
        for (std::size_t i {}; i < size; ++i) {
            table.id.emplace_back(i);
            table.age.emplace_back(i);
            table.occupation_id.emplace_back(i);
        }
        return table;
    }
}
