#pragma once

#include <veq/storage/column.hpp>
#include <string>

namespace veq {
    constexpr std::size_t TABLE_COLUMN_COUNT = 3;

    constexpr std::array<std::string, TABLE_COLUMN_COUNT> COLUMN_NAMES { "id", "age", "occupation_id" };

    struct Table {
        Column id {};
        Column age {};
        Column occupation_id {};
    };
}
