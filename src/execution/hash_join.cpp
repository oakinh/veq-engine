
#include "veq/execution/hash_join.hpp"

namespace veq {
    void build(std::span<const std::size_t> build_selection) {
        for (std::size_t idx : build_selection) {

        }
    }
    bool probe(std::span<const std::size_t> probe_selection) {

        // Must set rows_in_joined_buffer_ every time
    }
    void reset();
}