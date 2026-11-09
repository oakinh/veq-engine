# veq-engine

**A vectorized query execution engine written in C++23.**

This is an engine written for educational, and exploratory performance purposes. This engine is for my own benefit, to grasp fundamental database internal concepts and implementations, and come to understand them at a depth that allows thoughtful discussion of tradeoffs of architectural decisions, implementation differences, and performance impacts.

At this point in time, the project is not a complete DBMS, but only an execution engine. At some point in the future I will add a query parser and planner.

Not only does this contain implementation C++23 code, but it also contains:
- Architectural specification documents
- Various kinds of correctness tests
- Performance benchmarks and technical write-ups

## Motivation

I wanted to create a vectorized execution engine after my encounter with the weaknesses of row-oriented databases at GoodParty.org when I tried to run analytical queries on a Postgres instance holding the national voter file and added demographic information. 

The column-oriented architecture appealed to me, since I already had a fondness for performance optimization, and for the benefits of the struct-of-arrays and data-oriented design approach. Looking Glass Studio's Thief: The Dark Project (1998) is an old-school inspiration of this architectural ethos for me with its early Entity Component System.

## Current Implementation

The engine currently implements:

- **Column-oriented storage:** Contiguous `std::uint64_t` columns managed by a table.
- **Scan:** Iterates over table data in bounded batches of up to 1,024 rows.
- **Filter:** Evaluates predicates and produces selection vectors identifying matching rows.
- **Projection:** Selects column views while preserving the selection vector, without materializing the selected values.
- **COUNT aggregation:** Hash-based grouped counting, consuming either selected or unfiltered batches.
- **Custom aggregation hash table:** Open addressing with linear probing, automatic growth, and a maximum load factor of 70%.
- **Testing and benchmarking:** GoogleTest, Google Benchmark, sanitizer configurations, and GitHub Actions CI.

The current data model uses three `std::uint64_t` columns: `id`, `age`, and `occupation_id`.

## Architecture


                     veq-engine: Execution Architecture
```c++
+--------------------------------------------------------------+
|                           Table                              |
|                                                              |
|  id:             Column (std::vector<uint64_t>)              |
|  age:            Column (std::vector<uint64_t>)              |
|  occupation_id:  Column (std::vector<uint64_t>)              |
|                                                              |
|                   Owns column memory                         |
+-----------------------------+--------------------------------+
                              |
                              v
                     +----------------+
                     |      Scan      |
                     |  nextBatch()   |
                     +--------+-------+
                              |
                              v
                   +----------------------+
                   |     ColumnBatch      |
                   |                      |
                   | Non-owning views     |
                   | start_row, size      |
                   | max 1,024 rows       |
                   +----------+-----------+
                              |
                 +------------+-----------------+
                 |                              |
                 v                              |
        +------------------+                    |
        |      Filter      |                    |
        |    apply(...)    |                    |
        +--------+---------+                    |
                 |                              |
                 v                              |
        +-------------------------+             |
        |      SelectedBatch      |             |
        |                         |             |
        | Non-owning column views |             |
        | Selection vector        |             |
        | (global row indexes)    |             |
        +-------------+-----------+             |
                      |                         |
             +--------+------------+            |
             |                     |            |
             v                     |            |
      +-------------+              |            |
      | Projection  |              |            |
      |  apply()    |              |            |
      +------+------+              |            |
             |                     |            |
             v                     |            |
   +-------------------+           |            |
   |   ProjectedBatch  |           |            |
   |                   |           |            |
   | Column views      |           |            |
   | Selection vector  |           |            |
   +---------+---------+           |            |
             |                     |            |
             v                     |            |
   +-------------------+           |            |
   | TestMaterializer  |           |            |
   | (test utility)    |           |            |
   | Copies selected   |           |            |
   | values into       |           |            |
   | owned columns     |           |            |
   +-------------------+           |            |
                                   v            v
                             +----------------------+
                             |   CountAggregation   |
                             |                      |
                             | consume(SelectedBatch)|
                             | consume(ColumnBatch) |
                             +----------+-----------+
                                        |
                                        v
                             +----------------------+
                             |    CountHashTable    |
                             |                      |
                             | Open addressing      |
                             | Linear probing       |
                             | Key -> group count   |
                             +----------+-----------+
                                        |
                                        | finalize()
                                        v
                             +----------------------+
                             | CountAggregationResult|
                             |                      |
                             | keys_:   Column      |
                             | counts_: Column      |
                             +----------------------+
```

### Storage and Memory Layout
This isn't a storage engine, so my storage representation is overly simplistic. A `Table` is represented by a struct of arrays (SoA). Each array is a contiguous `Column` of homogenous data. For now, all data is typed as `std::uint64_t`, though I plan to expand this later.
This SoA approach allows me to execute mock-queries in a column-oriented fashion, and reap the performance benefits that come with that like:
- Cache locality over a column
- SIMD (single-instruction, multiple data) opportunities
- And perhaps later, compression

Applicable operators (ex: `Filter`, `Scan`) hold a private scratch buffer that is continually over-written with each processed batch. Otherwise, new arrays would have to be allocated and returned on every call. My approach dramatically reduces memory allocations, while introducing the need for careful awareness of memory lifetimes, and exactly when a buffer will be over-written.

The operator then returns a view (std::span) to the caller into this private scratch buffer. Additionally, a `ColumnView`, which is simply a pointer to the corresponding array in the original `Table` is returned and passed downstream as well.

Furthermore, this engine implements **late materialization**, so all each operator is passing downstream is a selection of global indices into the table. The table owns the data all the way until materialization occurs, and everyone else is simply viewing it in the meantime. No copy or move of actual table-data is performed until the data is _materialized_.

### Batch Processing
Data is processed in batches. With our streaming operators (discussed in "Operator Execution" below), data moves through the pipeline as it is scanned from the `Table`. Each batch is a maximum size of **1,024 rows**, which is currently a somewhat arbitrary placeholder value, to be compared and benchmarked in a future write-up.

Global indices are produced, which do not require any calculations against a base index as they would if they were offsets. This means they can be used directly against the original `Column` pointed to by the `ColumnView` as is.

Currently, each operator produces a batch specific to itself, `ColumnBatch`, `SelectedBatch`, `ProjectedBatch`, but this is a highly inflexible model, and does not easily facilitate variable operator execution order, and will be changed.

### Operator Execution
#### Execution Orchestration Overview
Each downstream operator indirectly pulls from the one upstream of it. Operators possess a member-function that takes input from the output of the operator upstream of it (except in the case of `Scan`, from the table itself), usually called `apply()`, that then returns that processed batch for the next downstream operator.

#### Blocking operators
Some operators, by nature of their operation, must possess _all_ the data relevant to the query before they can return results downstream.
Current blocking operators:
- `HashJoin`
- `CountAggregation`

Both of these operators also _materialize_ the views provided to them as part of their execution.

#### Streaming operators
Other operators do not require all the relevant data to begin their operation, and instead can receive, execute, and return data downstream in batches.
Current streaming operators:
- `Scan`
- `Filter`
- `Projection`

### Hash Aggregation
`CountHashTable`, is the custom-hash table implementation backing `CountAggregate`. This separation of concerns allows me to more easily support other group-by operators in the future. These classes are used to perform what would be a COUNT(*), GROUP BY query. For now, it only supports one integer key.

The custom hash-table implementation uses open-addressing, and linear-probing. It has a 70% capacity threshold before automatic growth is triggered, which is implemented as the allocation of a new buffer with iterative element moves if they are `noexcept` (as opposed to copies).

Being open-addressed and using linear probing means that the hash-table structure is an array of contiguous elements. Each index of the array holds exactly one `value_type` of the hash-table, as opposed to each element holding a linked-list of elements any time there is a collision.

Aggregation is a blocking operation, simply because it is intended to produce a finalized count over an entire query, and makes sense to be called after all other operators in the pipeline.

## Design Decisions and Tradeoffs

### Decision: Selection vectors over eager materialization
I chose to implement with selection vectors and late materialization, because I hoped it would reduce unnecessary early moves of data, and improve performance. I also hoped it would allow for less memory usage until it became necessary at the end of the query, being at the time for user consumption.

### Decision: Non-owning batch views and buffer reuse
Each operator writes it's output to a private scratch buffer that is continually re-used. Non-owning views (`std::span`'s) are returned to the caller, rather than incurring a move or copy of the selected indices on every operator execution function call. This means memory allocation for our indices during execution only has to happen once per operator.

Furthermore, the buffers are never resized, as that would require repeated reallocations if the size of the batch were to not be consistent, again improving performance. This does mean that callers need to be given a limited view of the scratch buffer (`std::span`'s `.subspan()` comes in handy here), lest they read beyond the bounds of what is valid data at that time.

### Decision: Hash aggregation's open addressing and linear probing
Our custom hash-table implementation forgoes STL's std::unordered_map's design choice of linked-lists and uses open addressing and linear probing (described briefly above in `Hash Aggregation`) with the aim of avoiding the poor-cache locality and dependent memory accesses of pointer chasing associated with linked lists. Instead, upon collision, the index is incremented until an open bucket is found (including if it has to wrap around back to the beginning of the array).

While we retain the performance benefit of our data being contiguous, it does mean that collisions can increase as the number of free array indices that would not result in a collision decrease. A planned future benchmark and technical writeup is on the performance characterists of std::unordered_map vs our current custom implementation, and other custom hash table implementations.

## Performance and Benchmarks

Benchmarks are implemented using Google Benchmark and cover:

- Baseline column scans and filtering
- Scan operator performance
- Predicate filtering
- Projection and materialization
- COUNT aggregation under varying row counts and group cardinalities

Existing investigations:

- [Scan, Filter, and Projection — First Pass](benchmarks/writeups/01_scan_filter_projection_first_pass.md)
- [COUNT Aggregation — First Pass](benchmarks/writeups/02_count_aggregation_first_pass.md)

### Findings
TBD

### Benchmark Reproduction

```bash
bash scripts/bench.sh
```

To run a subset of benchmarks, pass Google Benchmark arguments:

```bash
bash scripts/bench.sh --benchmark_filter=BM_CountAggregation
```

Benchmark results are workload- and hardware-dependent and should not be interpreted as general performance guarantees.

## Building and Running

### Requirements

The repository's provided development environment targets Linux, with CI running on Ubuntu 24.04.

Build requirements:

- C++23-compatible compiler (configured for Clang 19)
- CMake 3.22.1 or newer
- Ninja
- LLVM 19 toolchain, including `clang++-19`, `clang-tidy-19`, and `clang-format-19`
- LLVM libc++ and libc++abi development packages
- LLD linker
- Git and network access for initial dependency fetching

CMake FetchContent downloads the following dependencies automatically:

- GoogleTest 1.14.0
- Google Benchmark 1.8.3 (when benchmarks are enabled)

### Quick Start

```bash
git clone https://github.com/oakinh/veq-engine.git
cd veq-engine

# Install Ubuntu development dependencies
bash scripts/setup-ubuntu.sh

# Configure and build the default development preset
bash scripts/setup.sh

# Build and run tests with ASan/UBSan
bash scripts/test.sh

# Build and run benchmarks
bash scripts/bench.sh
```

The Ubuntu setup script requires sudo access and expects the configured APT repositories to provide the LLVM 19 packages.

### CMake Presets

| Preset | Purpose |
|---|---|
| `libcxx` | Debug build with Clang and libc++ |
| `asan` | AddressSanitizer and UndefinedBehaviorSanitizer |
| `tsan` | ThreadSanitizer |
| `release` | Optimized release build |
| `bench` | Optimized build with benchmarks enabled |

Build directly using CMake:

```bash
cmake --preset libcxx
cmake --build --preset libcxx -j
ctest --test-dir build/libcxx --output-on-failure
```

## Development Scripts

All scripts are located in [`scripts/`](scripts/).

| Script | Description |
|---|---|
| `setup-ubuntu.sh` | Installs Ubuntu build tools and the LLVM 19 development toolchain |
| `setup.sh` | Configures and builds the default `libcxx` development preset |
| `dev.sh` | Configures, builds, and runs the scratch executable; accepts an optional preset |
| `test.sh` | Builds and runs CTest, defaulting to the `asan` preset |
| `asan.sh` | Builds and runs the scratch executable under ASan/UBSan |
| `tsan.sh` | Builds and runs tests with ThreadSanitizer |
| `release.sh` | Builds the release configuration and runs tests |
| `bench.sh` | Builds and runs Google Benchmark |
| `format.sh` | Applies clang-format-19 to source, test, and benchmark files |
| `ccdb.sh` | Builds a selected preset and links its compilation database at the repository root |
| `use-preset-db.sh` | Switches the root `compile_commands.json` symlink to an existing preset's database |

The scripts use the CMake presets defined in [`CMakePresets.json`](CMakePresets.json).

## Testing and CI

The project uses GoogleTest for unit and operator tests, with CTest as the test runner.

GitHub Actions provides:

- Formatting checks using clang-format
- ASan/UBSan test builds
- ThreadSanitizer test builds
- Release-mode tests
- Benchmark compilation checks

The workflow runs on pull requests targeting `master`.

I have tests for each operator individually, as well as tests that run them together in a pipeline. These cover edge cases like empty tables, partial batches, non-contiguous selections, and ensuring no unnecessary copies of table data are made.

For hash aggregation, I also test correctness across many batches, collisions, rehashing, and preservation of all keys after growth.

## Current Limitations
Not currently supported:
- Multiple different data types
- Variable table schemas
- Null-value representation
- No SQL parsing or query optimizer
- SUM queries
- Joins
- Composable operators / pipeline scheduler
- Multi-threading

## Roadmap

### Planned
In rough order that I plan to tackle them:
- Hash join: Implement an inner equijoin (in progress)
- Execution pipelines: Improve operator interfaces and add instrumentation
- Performance profiling: Use `perf` and `strace` to investigate bottlenecks in an end-to-end query
- Column encoding: Implement RLE, dictionary encoding, or bit packing, and benchmark the memory and CPU tradeoffs.
- External sorting / spilling: Implement a memory-budgeted operator that can spill to disk when the working set exceeds available memory

### Longer-term Ideas
- Different operator interfaces (maybe type erasure)
- Experiment with custom allocators and memory arenas
- Support additional data types and null values
- Eventually add a query parser and planner

## Further Reading and References

[MonetDB/X100: Hyper-Pipelining Query Execution](https://ir.cwi.nl/pub/16497/)
[DuckDB -- Execution Format](https://duckdb.org/docs/current/internals/vector)
[The Design and Implementation of Modern Column-Oriented Database Systems](https://stratos.seas.harvard.edu/publications/design-and-implementation-modern-column-oriented-database-systems)
[CMU 15-445 -- Spring 2026](https://15445.courses.cs.cmu.edu/spring2026/schedule.html)
[ClickHouse -- Architecture Overview](https://github.com/ClickHouse/docs-reference/blob/master/docs/en/development/architecture.md)

Additional implementation details are available in [`specs/`](specs/).

