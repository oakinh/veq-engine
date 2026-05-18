#pragma once
#include <vector>
#include <veq/storage/batch.hpp>
#include "veq/execution/projection.hpp"

namespace veq::bench {
    class BenchMaterializer {
    public:
        void consume(const ProjectedBatch& batch) {
            for (std::size_t i{}; i < batch.columns.size(); ++i) {
                if (i == columns_.size()) {
                    columns_.push_back({});
                }
                for (std::size_t j{}; j < batch.selection_size; ++j) {
                    std::size_t selection_idx{batch.selection[j]};
                    columns_[i].push_back(batch.columns[i].data[selection_idx]);
                }
            }
        }

        const std::vector<std::vector<uint64_t>>& columns() const {
            return columns_;
        }
        void reset() {
            *this = {};
        }

        void clear() {
            for (auto& col : columns_) {
                col.clear();
            }
        }

    private:
        std::vector<std::vector<uint64_t>> columns_{};
    };
}

