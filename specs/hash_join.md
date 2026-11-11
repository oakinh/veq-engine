# Hash Join
### An inner equijoin
The hash join will be used to perform a join that returns records only when there is a match in both of the tables, and for every found match -- an inner equijoin. This means _maximum_ output cardinality will be M x N. 

This is a **blocking operator** as far as the build side is concerned. The build side is built in batches, but probing cannot commence until all build rows have been supplied and inserted into the internal hash table.

### Data structure
For the hash-table, we will initially use the STL provided `std::unordered_map`. Build sets with high-cardinality keys are expected to have heavily degraded performance. Later iterations may experiment with custom implementations.

`RecordId`: a zero-based global index into the original `Table` `Column`s.

Every record in the build side table will be inserted into our hash table. The join attribute is the `Key`, and the value is the `RecordId`. Duplicate keys will not be discarded, but their `RecordId` will instead be added to the hash table value.
```c++
class HashJoin {
public:
    
    HashJoin(
        std::span<const ColumnView> build_cols, 
        std::span<const ColumnView> probe_cols,
        std::size_t build_col_join_idx,
        std::size_t probe_col_join_idx
        );
    
    using Key = std::uint64_t;
    using RecordId = std::uint64_t;
    
    void build(std::span<const std::size_t> build_batch);
    bool probe(std::span<const std::size_t> probe_batch);
    void reset();
    
    std::span<const std::pair<RecordId, RecordId>> getJoinBuffer();
    std::span<const ColumnView> getBuildColumns();
    std::span<const ColumnView> getProbeColumns();

    
private:
    std::unordered_map<Key, std::vector<RecordId> hash_table_ {};
    std::size_t probe_selection_idx_ {};
    std::size_t build_match_idx_ {};
    
    // Buffer sizes
    std::size_t rows_in_joined_buffer_ {};
    std::size_t build_col_buffer_size_ {};
    std::size_t probe_col_buffer_size_ {};
    std::size_t build_col_join_idx_ {};
    std::size_t probe_col_join_idx_ {};
    
    // Output buffers
    std::array<std::pair<RecordId, RecordId>, MAX_BATCH_SIZE> joined_ids_buffer_ {};
    std::array<ColumnView, 3> build_columns_buffer_ {};
    std::array<ColumnView, 3> probe_columns_buffer_ {};
};

```

### The algorithm, in two phases
#### Build Phase
The smaller table should typically be provided during the build phase. We prefer the smaller table to ensure our build-side table fits into memory as often as possible. When it does not fit into memory, spill over behavior to disk is needed, which we will not be implementing in this pass.

#### The Probe Phase
We will iterate over the probe side table, looking up the join attributes (typically foreign key) in our hash-table. If found, for every RecordId in the value, we output a row (pair of RecordId's).

If a matching join has more than 1024 (batch size) records, then we will output a `true` bool indicating more rows remain. In this case, the same batch should be provided in the input. Internally when this happens, we store both the last processed `probe_selection_idx_` to index the provided `ProjectedBatch`, and the `build_match_idx_` to index the corresponding `std::vector<RecordId>` in the `hash_table_`.

We are opting for _late materialization_, so we will be outputting only the RecordIds themselves, and not copying the entire record to output.

### Public Member Functions
#### getJoinBuffer(), getBuildColumns(), getProbeColumns()
These member functions will return `std::span`s to the operator's private scratch buffers that hold the requested data. The entire view is guaranteed to be valid (internally done with the use of `.subspan()`). 

#### reset()
This method will wipe the hash-table, and the internal indices, `probe_selection_idx_` and `build_match_idx` used for mid-batch resumption

### Complexity and Performance Expectations
For a hash join with `B` build rows, `P` probe rows `D` distinct build keys, and `R` output rows:

- Build time: O(B) average
- Probe/output time: O(P + R) average
- Hash-table storage: O(B + D)
- Output buffer: O(1024)
- Worst case hashing: Likely much worse under pathological conditions

### How The Current System Will Need To Be Extended
- Add a join key column to the existing `Table` struct

### Call Order of Operators
DB 7.2 calls for implementing query planning. Operator call order at that time will not be static. For now, we will use this call order.
1. Scan
2. Filter
3. Project
4. Join (fed by two different pipelines)