# Aggregation V1
## Goals and Overview
- Single-key group by
  - One integer key, and one aggregate
- `COUNT(*)` first, `SUM` later
- Fundamentally, a hash-table-backed group-by operator
- Blocking operator, not a streaming operator. It consumes all batches and produces one final grouped result.

## Input
- A `ColumnView` for the key
- A `SelectedBatch` (which includes a selection vector. `selection` indexes are **global row indexes**)

## Memory Layout
```c++
class Aggregate { 
public:
    void consume(ColumnView, SelectionBatch);
    void consume(ColumnView, ColumnBatch);
    
    void finalize();
    const AggregationResult& getResult() const;
    
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

### `Aggregate` Overview
- We use a class for operator Aggregate so that it's easier in the future to support other group-by operations
- `Aggregate` owns the `CountHashTable`
- `add()` calls `CountHashTable::insert()`
- `CountAggregationResult` returned by `getResult()` is valid as long as `Aggregate` is in scope, and `finalize()` has been called.
- `consume()` does not modify `CountAggregationResult`, only `finalize()` does.
- `reset()` calls `CountHashTable::reset()`

#### consume()
- 

```c++
class CountHashTable {
public:
    void insert(Key);
    
    void reset();
    
    const std::vector<Bucket>& getBuckets() const;
    
private:
    void rehash(std::size_t count);
    
    std::vector<Bucket> buckets_;
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
- If load factor is exceeded, call `rehash(buckets_.size() * 2)`

#### reset()
- Simply changes all of the buckets to `occupied_ = false`. No other action is needed, because of a key invariant:
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
- The sum of all `Bucket.count_`s in `CountHashTable::buckets_` always equals the number of rows processed.

## Non-goals
- Preserving input order
- Streaming results
- `SUM`, `AVG`, etc.
- Values other than `std::uint64_t`
- Overflow handling of `std::unint64_t count_`

## V2 Ideas
- SoA layout
- Different probing techniques