#pragma once
#include <cstddef>
#include <cstdint>
#include <span>
#include <unordered_map>
#include <vector>
#include <ranges>
#include <cassert>

#include "veq/execution/projection.hpp"
#include "veq/storage/batch.hpp"

namespace veq {
    class HashJoin {
    public:
        HashJoin(
            std::span<const ColumnView> build_cols,
            std::span<const ColumnView> probe_cols,
            std::size_t build_col_join_idx,
            std::size_t probe_col_join_idx
        ) : build_col_buffer_size_(build_cols.size()),
            probe_col_buffer_size_(probe_cols.size()),
            build_col_join_idx_(build_col_join_idx),
            probe_col_join_idx_(probe_col_join_idx) {
            assert(build_col_join_idx < build_col_buffer_size_);
            assert(probe_col_join_idx < probe_col_buffer_size_);

            std::ranges::copy(build_cols, build_columns_buffer_.begin());
            std::ranges::copy(probe_cols, probe_columns_buffer_.begin());
        }

        using Key = std::uint64_t;
        using RecordId = std::size_t;

        // Execution and state
        void build(std::span<const std::size_t> build_selection);
        bool probe(std::span<const std::size_t> probe_selection);
        void reset();

        // Getters
        std::span<const std::pair<RecordId, RecordId>> getJoinBuffer() {
            return std::span{joined_ids_buffer_}.first(rows_in_joined_buffer_);
        }

        std::span<const ColumnView> getBuildColumns() {
            return std::span{ build_columns_buffer_ }.first(build_col_buffer_size_);
        }

        std::span<const ColumnView> getProbeColumns() {
            return std::span{ probe_columns_buffer_ }.first(probe_col_buffer_size_);
        }


    private:
        std::unordered_map<Key, std::vector<RecordId>> hash_table_ {};
        std::size_t last_probe_selection_idx_ {};
        std::size_t last_build_match_idx_ {};

        // Buffer sizes
        std::size_t rows_in_joined_buffer_ {};
        std::size_t build_col_buffer_size_ {};
        std::size_t probe_col_buffer_size_ {};

        // Join Key Indices
        std::size_t build_col_join_idx_ {};
        std::size_t probe_col_join_idx_ {};

        // Output buffers
        std::array<std::pair<RecordId, RecordId>, MAX_BATCH_SIZE> joined_ids_buffer_ {}; // { build_side, probe_side }
        std::array<ColumnView, TABLE_COLUMN_COUNT> build_columns_buffer_ {};
        std::array<ColumnView, TABLE_COLUMN_COUNT> probe_columns_buffer_ {};

    };
}

