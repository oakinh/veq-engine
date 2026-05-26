# Aggregation V1
## Goals and Overview
- Single-key group by
  - One integer key, and one aggregate
- `COUNT(*)` first, `SUM` later
- Fundamentally, a hash-table-backed group-by operator
- Blocking operator, not a streaming operator. It consumes all batches and produces one final grouped result.

## Input
- A `ColumnView` for the key
- Either: 
  - A `SelectedBatch` (which includes a selection vector. `selection` indexes are **global row indexes**)
  - A `ColumnBatch`
  - This enables the class to function even if a filter is or is not applied

## Memory Layout
```c++
class CountAggregation { 
public:
    void consume(ColumnView key_column, const SelectedBatch&);
    void consume(ColumnView key_column, const ColumnBatch&);
    
    void finalize();
    const CountAggregationResult& result() const;
    
    void reset();

private:
    CountHashTable hash_table_;
    CountAggregationResult result_;
}

struct CountAggregationResult {
    Column keys_;
    Column counts_;
};
```

### `CountAggregate` Overview
- We use a class for operator CountAggregation so that it's easier in the future to support other group-by operations
- `CountAggregation` owns the `CountHashTable`
- `add()` calls `CountHashTable::insert()`
- `CountAggregationResult` returned by `result()` is valid as long as `CountAggregation` is in scope, and `finalize()` has been called.
- `consume()` does not modify `CountAggregationResult`, only `finalize()` does.
- `reset()` calls `CountHashTable::reset()`, and clears result_.

#### CountAggregation Invariants
- `ColumnView` does not own memory and does not carry length. The caller is responsible for ensuring batch row indexes are valid for the underlying column.

#### CountAggregationResult (Output) Invariants
- `keys_.size()` == `counts_.size()`
- No duplicate keys_
- Order is unspecified. Input order is not preserved.
- If `finalize()`is called when no rows have been `consume()`'d since the last `reset()`, result_ contains a default constructed object with member objects of size 0.

#### consume()
- Processes the rows by calling `CountHashTable::insert()` repeatedly

#### finalize()
- Clears the old result if any, and materializes only occupied buckets.
- Calling this when no rows have been `consume()`'d since the last `reset()` does nothing
- Calling finalize is idempotent.

```c++
class CountHashTable {
public:
    using Key = std::uint64_t;
    
    void insert(Key);
    
    void reset();
    
    const std::vector<Bucket>& getBuckets() const;
    
private:
    void rehash(std::size_t new_capacity);
    
    std::vector<Bucket> buckets_ { 8 };
    std::size_t occupied_count_ = 0;
};
```
### CountHashTable Overview
- Open-addressing
- Linear probing

#### insert()
- Hashes the key to check buckets_.
  - If no `occupied` bucket with the key exists, the **first** found bucket where `!occupied_`, is used
    - The bucket's `key` is changed to our key, and the `count` is set to 1
  - If an `occupied` bucket with the key exists, we increment `count` by one.
- Checks the load factor, calls `rehash(buckets_.size() * 2)` if we exceed our max load factor.

#### reset()
- Simply changes all of the buckets to `occupied_ = false`, and `occupied_count_` to 0. 
- No other action is needed, because of a key invariant:
  - No bucket data is valid unless `occupied_ == true`.

#### growth / rehash()
Load factor = number of elements `occupied_` / size of array
- We double the size of the array, when load factor reaches 70%
  - This requires rehashing all the elements
- Process is:
  - Allocate a new table
  - Insert/move into it
  - Swap/commit
  - If any throw occurs, discard the new table
#### Invariants
- `buckets_.size() > 0`, always. We initialize it to 8.
- `buckets_` never exceeds load factor after `insert()` has completed
- `occupied_count_` always equals the number of `Bucket`s where `occupied_ == true`.
- After calling `reset()` no Bucket has `occupied_ == true`.
```c++
struct Bucket {
    std::uint64_t key_;
    std::uint64_t count_;
    bool occupied_ = false;
};
```
### Bucket Overview
- Key invariant: If `!occupied_`, no other `Bucket` data is valid, and should not be read.
- The sum of all _occupied_ `Bucket.count_`s in `CountHashTable::buckets_` always equals the number of rows processed since the last `reset()`

#### Invariants
- If `occupied_`, `count_ > 0`

## V1 Non-goals
- Preserving input order
- Streaming results
- `SUM`, `AVG`, etc.
- Values other than `std::uint64_t`
- Overflow handling of `std::uint64_t count_`

## V2 Ideas
- SoA layout
- Different probing techniques