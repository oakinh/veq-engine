# Hash Join
### An inner equijoin
The hash join will be used to perform a join that returns records only when there is a match in both of the tables, and for every found match -- an inner equijoin. This means the output will be M x N. 

This is a **blocking operator** as far as the build side is concerned. The build side cannot be handled in batches, and must receive the full input before any execution of joining may commence.

### Data structure
For the hash-table, we will initially use the STL provided std::unordered_map. Later iterations may experiment with custom implementations.

For every record in the build side table with a match in the probe side table of the join attributes (a primary key and foreign key for example), we will add it to our hash table. Duplicate keys will not be discarded, but their RecordId will instead be added to the std::vector.
```c++
class HashJoin {
public:
    HashJoin(int expected_build_rows) :
        expected_build_rows_(expected_build_rows)
    {};
    void build(const ProjectedBatch& build_batch);
    ProbeOutput probe(const ProjectedBatch& batch); 
    void resetHashTable();
private:
    using SelectedColumnView = std::pair<const ColumnView, std::vector<std::size_t>;
    using JoinedBatchBuffer = std::array<SelectedColumnView, MAX_BATCH_SIZE>;
    using JoinedBatch = std::span<SelectedColumnView>; // We will use .subspan if the joined_batch_buffer isn't fully utilized.
    
    struct ProbeOutput {
        JoinedBatch batch;
        int rows_remaining {}; // If 0, execution may continue by first calling loadProbeBatch again
    };
    
    int expected_build_rows_ {};
    std::unordered_map<Key, std::vector<RecordId> hash_table_ {};
    std::size_t batch_idx_ {};
    std::size_t record_idx_ {};
    
    JoinedBatchBuffer joined_batch_buffer_ {};
};

```

### The algorithm, in two phases
#### Build Phase
The smaller table should typically be provided during the build phase. We prefer the smaller table to ensure our build-side table fits into memory as often as possible. When it does not fit into memory, spill over behavior to disk is needed, which we will not be implementing in this pass.

#### The Probe Phase
As a check for correct interface usage, `probe()` will check that `hash_table_.size() == expected_build_rows_`.

We will then iterate over the probe side table, looking up the join attributes (typically foreign key) in our hash-table. If found, for every RecordId in the value, we output a row.

If a matching join has more than 1024 (batch size) records, then we will output in ProbeOutput the rows_remaining, and those rows can be supplied upon a subsequent call to `probe()`. Internally, we store both the last processed `batch_idx_` to index the provided `ProjectedBatch`, and the `record_idx_` to index the corresponding `std::vector<RecordId>` in the `hash_table_`.

We are opting for _late materialization_, so we will be outputing only the RecordIds themselves, and not copying the entire record.

#### How The Current System Will Need To Be Extended
- Ability to have more than one table
- Proper handling of a **blocking operator**

#### Call Order of Operators
DB 7.2 calls for implementing query planning. Operator call order at that time will not be static. For now, we will use this call order.
1. Scan
2. Filter
3. Project
4. Join