#pragma once

#include <vector>
#include <cstdint>

namespace veq {
    using Column = std::vector<std::uint64_t>;

    struct Table {
        Column id;
        Column age;
        Column occupation_id;
    };
}
