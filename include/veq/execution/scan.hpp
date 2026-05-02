#pragma once

#include <cassert>
#include <veq/storage/table.hpp>
#include <veq/storage/batch.hpp>
#include <iostream>

namespace veq {
    class Scan {
    public:
        using ColumnViews = std::array<const ColumnView, TABLE_COLUMN_COUNT>;

        explicit Scan(const Table& table, std::size_t batch_size = MAX_BATCH_SIZE)
            : table_{ table }
            , columns_ {
                ColumnView{ table.id.data() },
                ColumnView{ table.age.data() },
                ColumnView{ table.occupation_id.data() }
            }
            , batch_size_{ batch_size }
        {
            assert(batch_size_ <= MAX_BATCH_SIZE);
            assert(table.id.size() == table.age.size() && table.id.size() == table.occupation_id.size());
        }

        ColumnBatch nextBatch();
        bool hasNextBatch() const { return last_row_ < table_.id.size(); }
    private:
        const Table& table_;
        const ColumnViews columns_ {};
        std::size_t batch_size_ {};
        std::size_t last_row_ {};
    };
}
