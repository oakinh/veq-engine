#include <veq/execution/count_aggregation.hpp>

#include <cassert>
#include <functional>

namespace veq {
    bool CountHashTable::exceedsLoadFactor() const {
        assert(!buckets_.empty());
        return static_cast<double>(occupied_count_) / static_cast<double>(buckets_.size())
            > MAX_LOAD_FACTOR_;
    }

    void CountHashTable::insert(Key key) {
        // Hash the key first
        const auto hashed_key { std::hash<Key>{}(key) % buckets_.size() };
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
}