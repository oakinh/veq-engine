
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
        if (finished_last_batch) {
            assert(last_probe_selection_idx_ == 0);
            assert(last_build_match_idx_ == 0);
        }

        const ColumnView probe_join_key_col { probe_columns_buffer_[probe_col_join_idx_] };

        std::size_t output_buffer_idx {};

        for (std::size_t rel_probe_selection_idx { last_probe_selection_idx_ };
            rel_probe_selection_idx < probe_selection.size();
            ++rel_probe_selection_idx) {
            // These two loops are index-ception... Using buffer indexes to retrieve global table indexes...
            std::size_t global_probe_idx { probe_selection[rel_probe_selection_idx] };

            const auto it { hash_table_.find(probe_join_key_col.data[global_probe_idx]) };

            if (it != hash_table_.end()) {
                const auto& matched_indices { it->second };

                for (std::size_t rel_build_matched_idx { last_build_match_idx_ };
                    rel_build_matched_idx < matched_indices.size();
                    ++rel_build_matched_idx) {
                    if (output_buffer_idx == MAX_BATCH_SIZE) {
                        rows_in_joined_buffer_ = output_buffer_idx;

                        // Resumption tracking indices
                        last_build_match_idx_ = rel_build_matched_idx;
                        last_probe_selection_idx_ = rel_probe_selection_idx;
                        finished_last_batch = false;
                        return true;
                    };

                    std::size_t global_build_idx { matched_indices[rel_build_matched_idx] };

                    // Output buffer assignment
                    assert(it->first == probe_join_key_col.data[global_probe_idx]);
                    joined_ids_buffer_[output_buffer_idx] = { global_build_idx, global_probe_idx }; // { build_side, probe_side };
                    ++output_buffer_idx;

                    // if (output_buffer_idx == MAX_BATCH_SIZE) {
                    //     rows_in_joined_buffer_ = output_buffer_idx;
                    //
                    //     // Resumption tracking indices
                    //     last_build_match_idx_ = rel_build_matched_idx;
                    //     last_probe_selection_idx_ = rel_probe_selection_idx;
                    //     finished_last_batch = false;
                    //     return true;
                    // };
                }
            }
        }

        rows_in_joined_buffer_ = output_buffer_idx;

        // No resumption needed
        last_build_match_idx_ = 0;
        last_probe_selection_idx_ = 0;
        finished_last_batch = true;
        return false;
    }

    void HashJoin::reset() {
        rows_in_joined_buffer_ = 0;
        hash_table_.clear();
        build_col_buffer_size_ = 0;
        probe_col_buffer_size_ = 0;
    }
}
