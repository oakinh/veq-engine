#include <veq/storage/batch.hpp>
#include <veq/storage/table.hpp>
#include <veq/storage/batch.hpp>
#include <veq/execution/scan.hpp>

namespace veq {
    ColumnBatch Scan::nextBatch() {
        const std::size_t size { std::min(m_table.id.size() - m_last_row, m_batch_size) };
        std::vector scan_columns {
            ColumnView{ m_table.id.data() },
            ColumnView{ m_table.age.data() },
            ColumnView{ m_table.occupation_id.data() }
        };
        std::size_t new_start_row = m_last_row;
        m_last_row += size;

        return ColumnBatch{
            std::span{ scan_columns },
            new_start_row,
            size
        };
    }
}
