#pragma once
#include "table_builders.hpp"
#include <vector>
#include <veq/execution/scan.hpp>
#include <veq/storage/batch.hpp>

namespace veq::bench {
inline std::vector<ColumnBatch> buildColumnBatches(const Table& table) {
    std::vector<ColumnBatch> batches{};
    Scan scan{ table };

    while (scan.hasNextBatch()) {
        batches.push_back(scan.nextBatch());
    }

    return batches;
}
} // namespace veq::bench
