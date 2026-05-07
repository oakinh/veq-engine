# Vectorized Execution Query Engine

## Column
- Contiguous array of values. Stored densely with no nulls.
- Column owns it's memory.
`using Column = std::vector<uint64_t>`
- We're going to start with this type for now for v0. We'll later expand to supporting many types of data.
## Table
- Struct of arrays. Each array being a _column_.
- `Table` owns it's `Column`s
- Row identity is handled by the shared index across columns
```
struct Table {
    Column id;
    Column age;
    Column occupation_id;
}
```
### Invariants
- id.size() == age.size() == occupation_id.size()

## Operator Interface
```
class IOperator {
    
}
```


## Batch
- A set of rows, defined as views over multiple columns
- It does not own the column memory
```
struct Batch {
    std::array<const Column*, 3> columns {};
    std::vector<std::size_t> selection;
    std::size_t start_row;
    std::size_t size;
}
```
- `end_row` = `start_row` + `size`
- rows covered = `[start_row, end_row)`
- Batch capacity target = 1024. `size` <= 1024.
- Selection vector stores relative indices within the batch: 0 <= idx < size
- Selection vector is not guaranteed to be non-empty.

## Scan
- Reads the table, and produces `Batch`s, by returning views into existing memory

## Filter
- Produces a selection vector.
- Later we will conditionally produce a selection vector, or materialize.

## Projection
- Takes either a `ColumnBatch` or a `SelectedBatch`
- Preserves the selection vector.
- Does not own any actual table / column memory, only the views
- Performs no moves or copies
- Returns:
  - ```
    struct ProjectedBatch {
        std::span columns;
        std::span selection;
    }
```