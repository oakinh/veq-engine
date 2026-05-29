#include <veq/execution/count_aggregation.hpp>

#include <cassert>
#include <functional>

namespace veq {
    // Public member functions

    void CountHashTable::insert(Key key) {
        // Hash the key first
        std::size_t hashed_key { hashKey(key, buckets_.size()) };
        assert(hashed_key < buckets_.size() && "hashed_key not in bounds of buckets_");
        // Cap iterations, for loop doesn't maintain access in bounds, the body holds that responsibility...
        // the body supports wrap-around probing.
        for (std::size_t i {}; i < buckets_.size(); ++i) {
            std::size_t current_index { hashed_key };
            auto& bucket { buckets_[current_index] };

            if (!bucket.occupied_) {
                // Insert new key
                bucket.key_ = key;
                bucket.count_ = 1;
                bucket.occupied_ = true;

                if (exceedsLoadFactor()) {
                    rehash(buckets_.size() * GROWTH_FACTOR_);
                }
            } else if (bucket.key_ == key) {
                // Increment existing key
                assert(bucket.count_ > 0);
                assert(bucket.occupied_);
                ++bucket.count_;
            } else {
                ++current_index;
                continue;
            }

            break;
        }
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
        for (const auto& bucket : buckets_) {
            std::size_t hashed_key { hashKey(bucket.key_, new_buckets.size()) };
            Bucket& new_bucket { new_buckets[hashed_key] };
            new_bucket = std::move(bucket);
        }
        buckets_ = new_buckets;
    }

    bool CountHashTable::exceedsLoadFactor() const {
        assert(!buckets_.empty());
        return static_cast<double>(occupied_count_) / static_cast<double>(buckets_.size())
            > MAX_LOAD_FACTOR_;
    }

    std::size_t CountHashTable::hashKey(Key key, std::size_t bucket_size) const {
        assert(bucket_size > 0 && "Attempted to divide by zero");
        return std::hash<Key>{}(key) % bucket_size;
    }
}