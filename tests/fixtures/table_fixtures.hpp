#pragma once

#include <veq/storage/table.hpp>

namespace veq::test {
    inline veq::Table makeTinyTable() {
        return Table {
            .id = {
                1, 2, 3, 4, 5,
                6, 7, 8, 9, 10
            },
            .age = {
                25, 30, 25, 40, 30,
                25, 35, 40, 25, 30
            },
            .occupation_id = {
                100, 200, 100, 300, 200,
                100, 400, 300, 100, 200
            }
        };
    }
}