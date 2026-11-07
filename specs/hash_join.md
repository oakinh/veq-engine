# Hash Join
### An inner equijoin
The hash join will be used to perform a join that returns records only when there is a match in both of the tables, and for every found match -- an inner equijoin. This means _maximum_ output cardinality will be M x N. 

This is a **blocking operator** as far as the build side is concerned. The build side is built in batches, but probing cannot commence until all build rows have been supplied and inserted into the internal hash table.

### Data structure
For the hash-table, we will initially use the STL provided std::unordered_map. Later iterations may experiment with custom implementations.

Every record in the build side table will be inserted to our hash table. The join attribute is the Key, and the value is the RecordId. Duplicate keys will not be discarded, but their RecordId will instead be added to the hash table value.
```c++
class HashJoin {
public:
    using Key = std::uint64_t;
    using RecordId = std::uint64_t;
    
    void build(const ProjectedBatch& build_batch);
    ProbeOutput probe(const ProjectedBatch& batch);
    void resetHashTable();
    
    std::span<std::pair<RecordId, RecordId>> getJoinBuffer();
    std::span<ColumnView> getBuildColumns();
    std::span<ColumnView> getProbeColumns();
    
    ProbeOutput {
        int rows_in_buffer;
        int rows_remaining;
    }
    
private:
    std::unordered_map<Key, std::vector<RecordId> hash_table_ {};
    std::size_t batch_idx_ {};
    std::size_t record_idx_ {};
    
    // Output buffers
    std::array<std::pair<RecordId, RecordId>, MAX_BATCH_SIZE> joined_ids_buffer_ {};
    std::array<ColumnView> build_columns_buffer_ {};
    std::array<ColumnView> probe_columns_buffer_ {};
};

```

### The algorithm, in two phases
#### Build Phase
The smaller table should typically be provided during the build phase. We prefer the smaller table to ensure our build-side table fits into memory as often as possible. When it does not fit into memory, spill over behavior to disk is needed, which we will not be implementing in this pass.

#### The Probe Phase
As a check for correct interface usage, `probe()` 

We will then iterate over the probe side table, looking up the join attributes (typically foreign key) in our hash-table. If found, for every RecordId in the value, we output a row (pair of RecordId's).

If a matching join has more than 1024 (batch size) records, then we will output a positive integer representing the number of rows remaining. In this case, the same batch should be provided in the input. Internally when this happens, we store both the last processed `batch_idx_` to index the provided `ProjectedBatch`, and the `record_idx_` to index the corresponding `std::vector<RecordId>` in the `hash_table_`.

We are opting for _late materialization_, so we will be outputting only the RecordIds themselves, and not copying the entire record.

#### How The Current System Will Need To Be Extended
- Ability to have more than one table
- Proper handling of a **blocking operator**

#### Call Order of Operators
DB 7.2 calls for implementing query planning. Operator call order at that time will not be static. For now, we will use this call order.
1. Scan
2. Filter
3. Project
4. Join