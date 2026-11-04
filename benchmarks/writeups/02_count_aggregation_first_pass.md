# 02 - Count Aggregation Benchmark First Pass

The most interesting observation one can make about this benchmark, is that every case where the number of row counts equals the cardinality (every insert is a unique key), the items processed per second decreases significantly.

This is for a few reasons. Firstly, when every key is unique, our hash map has to do a lot more

2026-06-02T07:16:38-05:00
Running ./build/bench/benchmarks/veq_bench
Run on (24 X 4800 MHz CPU s)
CPU Caches:
L1 Data 48 KiB (x12)
L1 Instruction 32 KiB (x12)
L2 Unified 1280 KiB (x12)
L3 Unified 30720 KiB (x1)
Load Average: 0.66, 0.83, 0.96
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
----------------------------------------------------------------------------------------------------------------
Benchmark                                                      Time             CPU   Iterations UserCounters...
----------------------------------------------------------------------------------------------------------------
BM_CountAggregationFixedCardinality/1024/1                  2.36 us         2.36 us       298020 bytes_per_second=3.23427Gi/s items_per_second=434.097M/s
BM_CountAggregationFixedCardinality/1024/8                  2.37 us         2.37 us       296294 bytes_per_second=3.22529Gi/s items_per_second=432.891M/s
BM_CountAggregationFixedCardinality/1024/1024               5.26 us         5.26 us       133091 bytes_per_second=1.45156Gi/s items_per_second=194.825M/s
BM_CountAggregationFixedCardinality/65536/1                  112 us          112 us         6482 bytes_per_second=4.35727Gi/s items_per_second=584.823M/s
BM_CountAggregationFixedCardinality/65536/8                  111 us          111 us         6356 bytes_per_second=4.41102Gi/s items_per_second=592.038M/s
BM_CountAggregationFixedCardinality/65536/1024               122 us          122 us         6027 bytes_per_second=3.99567Gi/s items_per_second=536.289M/s
BM_CountAggregationFixedCardinality/65536/65536              369 us          369 us         1901 bytes_per_second=1.32204Gi/s items_per_second=177.441M/s
BM_CountAggregationFixedCardinality/16777216/1             28693 us        28688 us           25 bytes_per_second=4.3573Gi/s items_per_second=584.826M/s
BM_CountAggregationFixedCardinality/16777216/8             29625 us        29619 us           24 bytes_per_second=4.22024Gi/s items_per_second=566.431M/s
BM_CountAggregationFixedCardinality/16777216/1024          28883 us        28880 us           24 bytes_per_second=4.32828Gi/s items_per_second=580.931M/s
BM_CountAggregationFixedCardinality/16777216/65536         29940 us        29934 us           23 bytes_per_second=4.17582Gi/s items_per_second=560.469M/s
BM_CountAggregationFixedCardinality/16777216/16777216    1031171 us      1030959 us            1 bytes_per_second=124.156Mi/s items_per_second=16.2734M/s
