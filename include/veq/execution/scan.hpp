#pragma once

#include <cassert>
#include <veq/storage/batch.hpp>
#include <veq/storage/table/table.hpp>

namespace veq {
class Scan {
  public:
    explicit Scan(const Table& table, std::size_t batch_size = MAX_BATCH_SIZE)
        : table_{ table }, columns_{ ColumnView{ table.id.data() }, ColumnView{ table.age.data() },
                                     ColumnView{ table.occupation_id.data() } },
          batch_size_{ batch_size } {
        assert(batch_size_ <= MAX_BATCH_SIZE);
        assert(batch_size_ > 0);
        assert(table.id.size() == table.age.size() &&
               table.id.size() == table.occupation_id.size());
    }

    ColumnBatch nextBatch();
    bool hasNextBatch() const {
        return last_row_ < table_.id.size();
    }

  private:
    const Table& table_;
    // Operator owns the array of ColumnView so that we can return std::spans to it
    const ColumnViews columns_{};
    std::size_t batch_size_{};
    std::size_t last_row_{};
};
} // namespace veq
