#pragma once
#include <array>
#include <string>

namespace veq {
    inline constexpr std::size_t TABLE_COLUMN_COUNT = 3;

    inline constexpr std::array<std::string, TABLE_COLUMN_COUNT> COLUMN_NAMES_STR { "id", "age", "occupation_id" };

    enum class ColumnName {
        ID,
        AGE,
        OCCUPATION_ID,
        MAX_COLUMN_NAMES
    };
}