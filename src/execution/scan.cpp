#include <veq/storage/batch.hpp>
#include <veq/storage/table/table.hpp>
#include <veq/storage/batch.hpp>
#include <veq/execution/scan.hpp>

namespace veq {
    ColumnBatch Scan::nextBatch() {
        const std::size_t size { std::min(table_.id.size() - last_row_, batch_size_) };

        std::size_t new_start_row = last_row_;
        last_row_ += size;

        return ColumnBatch{
            std::span{ columns_ },
            new_start_row,
            size
        };
    }
}
