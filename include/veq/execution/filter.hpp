#pragma once

#include <veq/storage/batch.hpp>

namespace veq {
    enum class FilterOperator {
        LT,
        GT,
        EQ
    };

    struct FilterOperation {
        FilterOperator op {};
        std::uint64_t operand {};
    };

    class Filter {
    public:
        Filter(const Table& table) :
            m_table{ table }
        {}

        SelectedBatch apply(ColumnBatch& batch, FilterOperation operation);
    private:
        const Table& m_table;
    };
}