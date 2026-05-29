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
        static constexpr double MAX_LOAD_FACTOR_ { 0.70 };
        static constexpr std::size_t GROWTH_FACTOR_ { 2 };
        static constexpr std::size_t INITIAL_CAPACITY_ { 1024 };

        struct Bucket {
            Key key_ {};
            std::uint64_t count_ {};
            bool occupied_ = false;
        };

        std::vector<Bucket> buckets_ { INITIAL_CAPACITY_ };
        std::size_t occupied_count_ {};
        using Hasher = std::hash<Key>;

        std::size_t hashKey(Key key, std::size_t bucket_size) const;

        void rehash(std::size_t new_capacity);

        bool exceedsLoadFactor() const;

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
