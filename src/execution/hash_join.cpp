
#include "veq/execution/hash_join.hpp"

#include <functional>

namespace veq {
    void HashJoin::build(std::span<const std::size_t> build_selection) {
        // Key: join attribute, value: RecordId / global row index
        const ColumnView join_key_col { build_columns_buffer_[build_col_join_idx_] };

        for (std::size_t idx : build_selection) {
            hash_table_[join_key_col.data[idx]].emplace_back(idx);
        }
    }

    bool HashJoin::probe(std::span<const std::size_t> probe_selection) {
        const ColumnView probe_join_key_col { probe_columns_buffer_[probe_col_join_idx_] };

        std::size_t joined_buffer_idx {};

        for (std::size_t probe_selection_idx { last_probe_selection_idx_ };
            probe_selection_idx < probe_selection.size();
            ++probe_selection_idx) {
            // These two loops are index-ception... Using buffer indexes to retrieve global table indexes...
            std::size_t probe_idx { probe_selection[probe_selection_idx] };

            const auto it { hash_table_.find(probe_join_key_col.data[probe_idx]) };

            if (it != hash_table_.end()) {
                const auto& matched_indices { it->second };

                for (std::size_t build_matched_idx { last_build_match_idx_ };
                    build_matched_idx < matched_indices.size();
                    ++build_matched_idx) {
                    std::size_t build_idx { matched_indices[build_matched_idx] };

                    // Output buffer assignment
                    joined_ids_buffer_[joined_buffer_idx] = { build_idx, probe_idx }; // { build_side, probe_side };
                    ++joined_buffer_idx;

                    if (joined_buffer_idx == MAX_BATCH_SIZE) {
                        rows_in_joined_buffer_ = joined_buffer_idx;

                        // Resumption tracking indices
                        last_build_match_idx_ = build_idx;
                        last_probe_selection_idx_ = probe_idx;
                        return true;
                    };
                }
            }
        }

        rows_in_joined_buffer_ = joined_buffer_idx;

        // No resumption needed
        last_build_match_idx_ = 0;
        last_probe_selection_idx_ = 0;
        return false;
    }

    void HashJoin::reset() {
        rows_in_joined_buffer_ = 0;
        hash_table_.clear();
        build_col_buffer_size_ = 0;
        probe_col_buffer_size_ = 0;
    }
}
