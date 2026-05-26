#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>

#include <veq/storage/column.hpp>
#include <veq/storage/batch.hpp>


namespace veq {
    class CountHashTable {
    public:
        using Key = std::uint64_t;

        void insert(Key key);

        void reset();

    private:
        struct Bucket {
            Key key_;
            std::uint64_t count_;
            bool occupied_ = false;
        };

        void rehash(std::size_t new_capacity);

        std::vector<Bucket> buckets_ {};
        std::size_t occupied_count_ {};
    };

    struct CountAggregationResult {
        Column keys_;
        Column counts_;
    };

    class CountAggregation {
    public:
        void consume(ColumnView key_column, const SelectedBatch&);
        void consume(ColumnView key_column, const ColumnBatch&);

        void finalize();
        const CountAggregationResult& result() const;

        void reset();

    private:
        CountHashTable hash_table_ {};
        CountAggregationResult result_ {};
    };
}
