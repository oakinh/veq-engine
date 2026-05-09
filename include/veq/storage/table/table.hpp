#pragma once

#include <string>
#include <veq/storage/column.hpp>

namespace veq {
struct Table {
    Column id{};
    Column age{};
    Column occupation_id{};
};
} // namespace veq
