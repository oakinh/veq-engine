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
    void build(ProjectedBatch& build_batch);
    JoinedBatch probe(ProjectedBatch& probe_batch); // Should not be called before build side has received all batches
private:
    using SelectedColumnView = std::pair<const ColumnView, std::vector<std::size_t>;
    using JoinedBatchBuffer = std::array<SelectedColumnView, MAX_BATCH_SIZE>;
    using JoinedBatch = std::span<SelectedColumnView> selection; // We will use .subspan if the joined_batch_buffer isn't fully utilized.
    
    std::unordered_map<Key, std::vector<RecordId> hash_table {};
    
    JoinedBatchBuffer joined_batch_buffer {};
};

```

### The algorithm, in two phases
#### Build Phase
The smaller table is selected to use in the construction of a hash-table. We choose the smaller table (while providing an override) to ensure our build-side table fits into memory as often as possible. When it does not fit into memory, spill over behavior to disk is needed, which we will not be implementing in this pass.

#### The Probe Phase
We will then iterate over the probe side table, looking up the join attributes (typically foreign key) in our hash-table. If found, for every RecordId in the value, we output a row.
We are opting for _late materialization_, so we will be outputing only the RecordIds themselves, and not copying the entire record.

#### How The Current System Will Need To Be Extended
- More than one table will need to be added

#### Call Order of Operators
DB 7.2 calls for implementing query planning. Operator call order at that time will not be static. For now, we will use this call order.
1. Scan
2. Filter
3. Project
4. Join