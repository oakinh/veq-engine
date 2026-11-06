#pragma once
#include <veq/storage/batch.hpp>
#include <veq/storage/column.hpp>

namespace veq {

    /* TODO: DB 7.2 will likely require moving away from these isolated operator-named batches.
     * ProjectedBatch and SelectedBatch are structurally equivalent
     */
struct ProjectedBatch {
    std::span<const ColumnView> columns{};
    std::span<const std::size_t> selection{};
    std::size_t selection_size{};
};

class Projection {
    // This operator is responsible for modifying (excluding) what columns downstream operators can see
    // In a table with id, age, and occupation, SELECT id, age would then require this operator to only output those columns...
    // ...and to exclude the occupation column.
    // This saves us from materializing or performing other work on columns we don't actually want.
  public:
    Projection() = default;

    // TODO: This might be a terrible pattern. Seems error prone when used in things like
    // veq::test::runPipeline
    void setTargetColumns(const ColumnViews& columns) {
        columns_ = columns;
    };

    // No new buffer is allocated, no values are read or copied. Only a different view of data is produced.
    ProjectedBatch apply(const SelectedBatch& input);

  private:
    // Doesn't change batch to batch
    ColumnViews columns_{};
};
} // namespace veq