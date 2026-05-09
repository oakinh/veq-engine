#pragma once

#include <veq/storage/table/table.hpp>

namespace veq::test {
inline veq::Table makeTinyTable() {
    // Size 10
    return Table{.id = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                 .age = {25, 30, 25, 40, 30, 25, 35, 40, 25, 30},
                 .occupation_id = {100, 200, 100, 300, 200, 100, 400, 300, 100, 200}};
}

inline Table makeSizeOneTable() {
    return Table{.id = {1}, .age = {12}, .occupation_id = {25}};
}
} // namespace veq::test