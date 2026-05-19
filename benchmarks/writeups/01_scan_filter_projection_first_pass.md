# 01 - Scan, Filter, Projection, Benchmark-Materialization First Pass
This is the initial benchmark of our first-pass implementation of Scan, Filter, Projection, and benchmark-Materialization.

The baselines are not made to be an exact comparison to their actual implementation counterparts. 
The baselines instead serve as to inform what the hardware is capable of for some of our core operations.

We can see from the results that my implementation does not have any _glaring_ performance issues, like accidental copies.

It is also very interesting to notice the significant dropoff in bytes/second processed once our working set exceeds effective cache capacity.
---
Run on (24 X 4800 MHz CPU s)
CPU Caches:
L1 Data 48 KiB (x12)
L1 Instruction 32 KiB (x12)
L2 Unified 1280 KiB (x12)
L3 Unified 30720 KiB (x1)
Load Average: 0.61, 0.96, 1.28
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
-------------------------------------------------------------------------------------------------------------
Benchmark                                                   Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------------------------------
BM_Baseline_ColumnScanSum/1024                          0.077 us        0.077 us      9235700 bytes_per_second=98.7575Gi/s items_per_second=13.255G/s
BM_Baseline_ColumnScanSum/2048                          0.152 us        0.152 us      4405852 bytes_per_second=100.132Gi/s items_per_second=13.4395G/s
BM_Baseline_ColumnScanSum/4096                          0.309 us        0.308 us      2278418 bytes_per_second=98.9415Gi/s items_per_second=13.2797G/s
BM_Baseline_ColumnScanSum/8192                          0.613 us        0.612 us      1120480 bytes_per_second=99.6561Gi/s items_per_second=13.3756G/s
BM_Baseline_ColumnScanSum/16384                          1.18 us         1.18 us       601457 bytes_per_second=103.857Gi/s items_per_second=13.9395G/s
BM_Baseline_ColumnScanSum/32768                          2.26 us         2.26 us       309754 bytes_per_second=108.244Gi/s items_per_second=14.5282G/s
BM_Baseline_ColumnScanSum/65536                          4.53 us         4.52 us       154813 bytes_per_second=107.934Gi/s items_per_second=14.4866G/s
BM_Baseline_ColumnScanSum/131072                         10.9 us         10.9 us        63176 bytes_per_second=89.4362Gi/s items_per_second=12.0039G/s
BM_Baseline_ColumnScanSum/262144                         27.2 us         27.2 us        25683 bytes_per_second=71.7275Gi/s items_per_second=9.6271G/s
BM_Baseline_ColumnScanSum/524288                         56.7 us         56.7 us        11925 bytes_per_second=68.9136Gi/s items_per_second=9.24943G/s
BM_Baseline_ColumnScanSum/1048576                         118 us          118 us         6010 bytes_per_second=66.0736Gi/s items_per_second=8.86825G/s
BM_Baseline_ColumnScanSum/2097152                         264 us          264 us         2663 bytes_per_second=59.2723Gi/s items_per_second=7.95539G/s
BM_Baseline_ColumnScanSum/4194304                        1101 us         1101 us          630 bytes_per_second=28.3917Gi/s items_per_second=3.81067G/s
BM_Baseline_ColumnScanSum/8388608                        2675 us         2675 us          260 bytes_per_second=23.367Gi/s items_per_second=3.13627G/s
BM_Baseline_ColumnScanSum/16777216                       5424 us         5423 us          128 bytes_per_second=23.0505Gi/s items_per_second=3.09378G/s
BM_Baseline_FilterSelectionVector/1024/1                0.460 us        0.460 us      1516345 bytes_per_second=16.5878Gi/s items_per_second=2.22637G/s
BM_Baseline_FilterSelectionVector/1024/50               0.326 us        0.326 us      2143854 bytes_per_second=23.432Gi/s items_per_second=3.14499G/s
BM_Baseline_FilterSelectionVector/1024/99               0.306 us        0.306 us      2319301 bytes_per_second=24.9124Gi/s items_per_second=3.34368G/s
BM_Baseline_FilterSelectionVector/65536/1                35.8 us         35.8 us        19590 bytes_per_second=13.6466Gi/s items_per_second=1.83162G/s
BM_Baseline_FilterSelectionVector/65536/50               20.2 us         20.2 us        34211 bytes_per_second=24.2267Gi/s items_per_second=3.25166G/s
BM_Baseline_FilterSelectionVector/65536/99               18.9 us         18.9 us        37065 bytes_per_second=25.8204Gi/s items_per_second=3.46555G/s
BM_Baseline_FilterSelectionVector/16777216/1            11721 us        11713 us           55 bytes_per_second=10.6721Gi/s items_per_second=1.43238G/s
BM_Baseline_FilterSelectionVector/16777216/50            8726 us         8723 us           76 bytes_per_second=14.3294Gi/s items_per_second=1.92326G/s
BM_Baseline_FilterSelectionVector/16777216/99            7941 us         7940 us           88 bytes_per_second=15.7436Gi/s items_per_second=2.11307G/s
BM_Baseline_ProjectWithSelectionVector/1024/1           0.147 us        0.147 us      4761804 bytes_per_second=129.822Gi/s items_per_second=6.96979G/s
BM_Baseline_ProjectWithSelectionVector/1024/4           0.038 us        0.038 us     18593096 bytes_per_second=124.684Gi/s items_per_second=6.69393G/s
BM_Baseline_ProjectWithSelectionVector/1024/16          0.010 us        0.010 us     68355082 bytes_per_second=115.786Gi/s items_per_second=6.21621G/s
BM_Baseline_ProjectWithSelectionVector/65536/1           17.3 us         17.3 us        40544 bytes_per_second=70.5758Gi/s items_per_second=3.78901G/s
BM_Baseline_ProjectWithSelectionVector/65536/4           5.15 us         5.15 us       131598 bytes_per_second=59.2098Gi/s items_per_second=3.1788G/s
BM_Baseline_ProjectWithSelectionVector/65536/16          1.75 us         1.75 us       407589 bytes_per_second=43.6918Gi/s items_per_second=2.34568G/s
BM_Baseline_ProjectWithSelectionVector/16777216/1       12492 us        12474 us           59 bytes_per_second=25.0512Gi/s items_per_second=1.34493G/s
BM_Baseline_ProjectWithSelectionVector/16777216/4        6240 us         6235 us          110 bytes_per_second=12.5291Gi/s items_per_second=672.65M/s
BM_Baseline_ProjectWithSelectionVector/16777216/16       4580 us         4541 us          165 bytes_per_second=4.30141Gi/s items_per_second=230.93M/s
BM_FilterAge/1024/1                                     0.308 us        0.306 us      2326785 bytes_per_second=24.9211Gi/s items_per_second=3.34486G/s
BM_FilterAge/1024/50                                    0.296 us        0.295 us      2426368 bytes_per_second=25.8516Gi/s items_per_second=3.46974G/s
BM_FilterAge/1024/99                                    0.178 us        0.178 us      4083143 bytes_per_second=42.8972Gi/s items_per_second=5.75756G/s
BM_FilterAge/65536/1                                     17.7 us         17.7 us        39293 bytes_per_second=27.6021Gi/s items_per_second=3.70469G/s
BM_FilterAge/65536/50                                    17.8 us         17.8 us        39892 bytes_per_second=27.3811Gi/s items_per_second=3.67503G/s
BM_FilterAge/65536/99                                    10.9 us         10.9 us        63861 bytes_per_second=44.7583Gi/s items_per_second=6.00736G/s
BM_FilterAge/16777216/1                                  7609 us         7609 us           90 bytes_per_second=16.4289Gi/s items_per_second=2.20505G/s
BM_FilterAge/16777216/50                                 7293 us         7285 us           90 bytes_per_second=17.1588Gi/s items_per_second=2.30301G/s
BM_FilterAge/16777216/99                                 6620 us         6620 us          100 bytes_per_second=18.8834Gi/s items_per_second=2.53448G/s
BM_ProjectAndMaterialize/1024/1                          1.44 us         1.44 us       495523 bytes_per_second=5.24145Gi/s items_per_second=703.496M/s selected_rows=1.013k selectivity=0.989258
BM_ProjectAndMaterialize/1024/50                        0.768 us        0.768 us       928257 bytes_per_second=4.8521Gi/s items_per_second=651.238M/s selected_rows=500 selectivity=0.488281
BM_ProjectAndMaterialize/1024/99                        0.014 us        0.014 us     48098750 bytes_per_second=5.21737Gi/s items_per_second=700.264M/s selected_rows=10 selectivity=9.76562m
BM_ProjectAndMaterialize/65536/1                         90.9 us         90.9 us         7690 bytes_per_second=5.31922Gi/s items_per_second=713.933M/s selected_rows=64.88k selectivity=0.98999
BM_ProjectAndMaterialize/65536/50                        45.7 us         45.7 us        15095 bytes_per_second=5.34198Gi/s items_per_second=716.988M/s selected_rows=32.75k selectivity=0.499725
BM_ProjectAndMaterialize/65536/99                        1.01 us         1.01 us       705300 bytes_per_second=4.83077Gi/s items_per_second=648.374M/s selected_rows=655 selectivity=9.99451m
BM_ProjectAndMaterialize/16777216/1                     25272 us        25265 us           26 bytes_per_second=4.89805Gi/s items_per_second=657.405M/s selected_rows=16.6094M selectivity=0.99
BM_ProjectAndMaterialize/16777216/50                    14481 us        14473 us           46 bytes_per_second=4.31845Gi/s items_per_second=579.612M/s selected_rows=8.3886M selectivity=0.5
BM_ProjectAndMaterialize/16777216/99                      653 us          653 us          899 bytes_per_second=1.91415Gi/s items_per_second=256.912M/s selected_rows=167.772k selectivity=9.99999m
BM_ScanColumns/1024                                     0.074 us        0.074 us      9573553 bytes_per_second=103.48Gi/s items_per_second=13.8889G/s
BM_ScanColumns/2048                                     0.143 us        0.143 us      4982721 bytes_per_second=106.548Gi/s items_per_second=14.3006G/s
BM_ScanColumns/4096                                     0.282 us        0.281 us      2480894 bytes_per_second=108.485Gi/s items_per_second=14.5605G/s
BM_ScanColumns/8192                                     0.630 us        0.629 us      1102090 bytes_per_second=96.9983Gi/s items_per_second=13.0189G/s
BM_ScanColumns/16384                                     1.24 us         1.24 us       567079 bytes_per_second=98.2267Gi/s items_per_second=13.1838G/s
BM_ScanColumns/32768                                     2.48 us         2.48 us       282740 bytes_per_second=98.4446Gi/s items_per_second=13.213G/s
BM_ScanColumns/65536                                     5.03 us         5.03 us       137342 bytes_per_second=97.0691Gi/s items_per_second=13.0284G/s
BM_ScanColumns/131072                                    10.7 us         10.7 us        65353 bytes_per_second=91.4474Gi/s items_per_second=12.2739G/s
BM_ScanColumns/262144                                    29.8 us         29.8 us        23660 bytes_per_second=65.4897Gi/s items_per_second=8.78988G/s
BM_ScanColumns/524288                                    61.8 us         61.8 us        11175 bytes_per_second=63.1966Gi/s items_per_second=8.48211G/s
BM_ScanColumns/1048576                                    127 us          127 us         5125 bytes_per_second=61.5494Gi/s items_per_second=8.26102G/s
BM_ScanColumns/2097152                                    382 us          381 us         2316 bytes_per_second=40.9808Gi/s items_per_second=5.50035G/s
BM_ScanColumns/4194304                                   1170 us         1169 us          549 bytes_per_second=26.7217Gi/s items_per_second=3.58653G/s
BM_ScanColumns/8388608                                   2789 us         2789 us          228 bytes_per_second=22.4096Gi/s items_per_second=3.00776G/s
BM_ScanColumns/16777216                                  5727 us         5726 us          120 bytes_per_second=21.8312Gi/s items_per_second=2.93014G/s