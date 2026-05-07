#include <veq/execution/projection.hpp>
#include <cassert>

namespace veq {

    ProjectedBatch Projection::apply(const SelectedBatch& input) {
        // ProjectedBatch batch {};
        // std::size_t i {};
        // for (const auto& col : selected_columns) {
        //     // Use the enum as an index into the column view from input batch
        //     assert(i < input.columns.size());
        //     std::size_t column_id { static_cast<std::size_t>(col) };
        //     columns_[i] = input.columns[column_id];
        //     selected_column_ids_[i] = column_id;
        //     ++i;
        // }
        // columns_size_ = i + 1;
        return {
            .columns=columns_,
            .selection=input.selection,
            .size=input.size
        };
    }
}