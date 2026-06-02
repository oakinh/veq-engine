#include <veq/execution/count_aggregation.hpp>
#include <veq/storage/table/table.hpp>

#include <cassert>
#include <functional>
#include <iostream>

namespace veq {
    // CountAggregation
    // Public member functions

    void CountAggregation::consume(const SelectedBatch& batch, std::size_t key_column_idx) {
        const auto& [columns, selection, size] = batch;

        assert(size <= selection.size());
        // Reminder: selection holds global indexes into columns
        for (std::size_t i {}; i < size; ++i) {
            auto column_idx { selection[i] };

            assert(key_column_idx < columns.size() && "key_column_idx not in bounds");
            const auto& key_column_view { columns[key_column_idx] };

            hash_table_.insert(key_column_view.data[column_idx]);
        }
    }

    void CountAggregation::consume(const ColumnBatch& batch, std::size_t key_column_idx) {
        auto [ columns, start_row, size ] { batch };
        for (std::size_t i { start_row }; i < size + start_row; ++i) {
            const auto& key_column_view { batch.columns[key_column_idx] };
            hash_table_.insert(key_column_view.data[i]);
        }
    }

    void CountAggregation::reset() {
        hash_table_.reset();
        result_.keys_.clear();
        result_.counts_.clear();
    }

    void CountAggregation::finalize() {
        const auto& buckets { hash_table_.getBuckets() };
        std::size_t occupied_count { hash_table_.getOccupiedCount() };

        result_.keys_.reserve(occupied_count);
        result_.counts_.reserve(occupied_count);

        for (std::size_t i {}; i < buckets.size(); ++i) {
            const auto& bucket { buckets[i] };
            if (bucket.occupied_) {
                result_.keys_.emplace_back(bucket.key_);
                result_.counts_.emplace_back(bucket.count_);
                // result_.keys_[i] = bucket.key_;
                // result_.counts_[i] = bucket.count_;
            }
        }
        assert(result_.keys_.size() == result_.counts_.size());
        assert(result_.keys_.size() == occupied_count);
    }

    // CountHashTable
    // Public member functions

    void CountHashTable::insert(Key key) {
        // Hash the key first
        std::size_t hashed_key { hashKey(key, buckets_.size()) };
        assert(hashed_key < buckets_.size() && "hashed_key not in bounds of buckets_");
        // Cap iterations, for loop doesn't maintain access in bounds, the body holds that responsibility...
        // the body supports wrap-around probing.


        for (std::size_t iterations {}, current_index { hashed_key };
            iterations < buckets_.size();
            ++iterations, current_index = (current_index + 1) % buckets_.size()) {

            assert(current_index < buckets_.size());
            auto& bucket { buckets_[current_index] };

            if (!bucket.occupied_) {
                // Insert new key
                bucket.key_ = key;
                bucket.count_ = 1;
                bucket.occupied_ = true;
                ++occupied_count_;

                if (exceedsLoadFactor()) {
                    rehash(buckets_.size() * GROWTH_FACTOR_);
                }
                return;
            } else if (bucket.key_ == key) {
                // Increment existing key
                assert(bucket.count_ > 0);
                assert(bucket.occupied_);
                ++bucket.count_;
                return;
            } else {
                continue;
            }

            break;
        }
        throw std::logic_error("Failed to insert key");
    }

    void CountHashTable::reset() {
        for (auto& bucket : buckets_) {
            bucket.occupied_ = false;
            bucket.count_ = 0;
        }
        occupied_count_ = 0;
    }

    // Private member functions

    void CountHashTable::rehash(std::size_t new_capacity) {
        // V1 doesn't worry about exception safety since we are only using fundamental types
        std::vector<Bucket> new_buckets { new_capacity };
        new_buckets.resize(new_capacity);

        for (auto& bucket : buckets_) {
            if (bucket.occupied_) {
                std::size_t hashed_key { hashKey(bucket.key_, new_buckets.size()) };
                Bucket& new_bucket { new_buckets[hashed_key] };
                new_bucket = std::move(bucket);
            }
        }
        buckets_ = new_buckets;
    }

    bool CountHashTable::exceedsLoadFactor() const {
        assert(!buckets_.empty());
        return static_cast<double>(occupied_count_) / static_cast<double>(buckets_.size())
            > MAX_LOAD_FACTOR_;
    }

    std::size_t CountHashTable::hashKey(Key key, std::size_t bucket_size) {
        assert(bucket_size > 0 && "Attempted to divide by zero");
        return (std::hash<Key>{}(key) % bucket_size);
    }
}