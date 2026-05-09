#include <cassert>
#include <veq/execution/projection.hpp>

namespace veq {

ProjectedBatch Projection::apply(const SelectedBatch& input) {
    return {.columns = columns_, .selection = input.selection, .selection_size = input.size};
}
} // namespace veq