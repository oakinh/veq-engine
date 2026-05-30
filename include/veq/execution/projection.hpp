#pragma once
#include <veq/storage/batch.hpp>
#include <veq/storage/column.hpp>

namespace veq {

struct ProjectedBatch {
    std::span<const ColumnView> columns{};
    std::span<const std::size_t> selection{};
    std::size_t selection_size{};
};

class Projection {
  public:
    Projection() = default;

    // TODO: This might be a terrible pattern. Seems error prone when used in things like veq::test::runPipeline
    void setTargetColumns(const ColumnViews& columns) {
        columns_ = columns;
    };

    ProjectedBatch apply(const SelectedBatch& input);

  private:
    // Doesn't change batch to batch
    ColumnViews columns_{};
};
} // namespace veq