#include <veq/storage/column.hpp>

#include "veq/storage/batch.hpp"

namespace veq {
    struct CountAggregationResult;
    class CountHashTable;

    class CountAggregation {
    public:
        void consume(ColumnView key_column, const SelectedBatch&);
        void consume(ColumnView key_column, const ColumnBatch&);

        void finalize();
        const CountAggregationResult& getResult() const;

        void reset();

    private:
        CountHashTable hash_table_ {};
        CountAggregationResult result_ {};
    };

    struct CountAggregationResult {
        Column keys_;
        Column counts_;
    };

    class CountHashTable {

    };
}
