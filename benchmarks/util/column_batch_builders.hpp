#pragma once
#include <veq/storage/batch.hpp>
#include <veq/execution/scan.hpp>
#include <vector>
#include "table_builders.hpp"

namespace veq::bench {
    std::vector<ColumnBatch> buildBatches(const Table& table) {
        std::vector<ColumnBatch> batches {};
        Scan scan { table };

        while (scan.hasNextBatch()) {
            batches.push_back(scan.nextBatch());
        }

        return batches;
    }
}
