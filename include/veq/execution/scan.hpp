#pragma once

#include <cassert>
#include <veq/storage/table.hpp>
#include <veq/storage/batch.hpp>

namespace veq {
    class Scan {
    public:
        ColumnBatch nextBatch();

        explicit Scan(const Table& table, std::size_t batch_size = MAX_BATCH_SIZE)
            : m_table{ table }
            , m_batch_size{ batch_size }
        {
            assert(m_batch_size <= MAX_BATCH_SIZE);
        }

    private:
        const Table& m_table;
        std::size_t m_batch_size {};
        std::size_t m_last_row {};


    };
}
