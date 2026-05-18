#pragma once
#include <vector>
#include <veq/storage/batch.hpp>
#include "veq/execution/projection.hpp"

namespace veq::bench {
    class BenchMaterializer {
    public:
        void prepare(std::size_t column_count, std::size_t row_capacity) {
            columns_.clear();
            columns_.resize(column_count);

            for (auto& col : columns_) {
                col.reserve(row_capacity);
            }
        }

        void consume(const ProjectedBatch& batch) {
            assert(columns_.size() == batch.columns.size());
            for (std::size_t i{}; i < batch.columns.size(); ++i) {
                for (std::size_t j{}; j < batch.selection_size; ++j) {
                    std::size_t selection_idx { batch.selection[j] };
                    auto val { batch.columns[i].data[selection_idx] };
                    columns_[i].push_back(val);
                    checksum_ += val;
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
            checksum_ = 0;
        }

        std::size_t rowCount() const {
            return columns_.empty() ? 0 : columns_[0].size();
        }

        std::uint64_t checksum() const { return checksum_; }

    private:
        std::vector<std::vector<uint64_t>> columns_{};
        std::uint64_t checksum_ {};
    };
}

