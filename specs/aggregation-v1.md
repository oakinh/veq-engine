# Aggregation V1
## Goals and Overview
- Single-key group by
  - One integer key, and one aggregate
- `COUNT(*)` first, `SUM` later
- Fundamentally, a hash-table-backed group-by operator
- Blocking operator, not a streaming operator. It consumes all batches and produces one final grouped result

## Memory Layout
```c++
class Aggregate { 
public:
  void add(ColumnView, SelectionBatch);

private:
  CountHashTable hash_table;
}
```

```c++
class CountHashTable {
public:
    void insert(Key);
    
    void reset();
    
    const std::vector<Bucket>& getBuckets() const;
    
private:
    std::vector<Bucket> buckets_;
};
```

## Non-goals
- Preserving input order
- Streaming results
- `SUM`, `AVG`, etc.
- Values other than `std::uint64_t`

## Input
- A `ColumnView` for the key
- A `SelectedBatch` (which includes a selection vector. `selection` indexes are **global row indexes**)

## V2 Ideas
- SoA layout
- Different probing techniques