#pragma once

#include <veq/storage/column.hpp>
#include <string>

namespace veq {
    struct Table {
        Column id {};
        Column age {};
        Column occupation_id {};
    };
}
