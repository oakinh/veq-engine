# CMake generated Testfile for 
# Source directory: /home/nikaoh/workspace/github.com/oakinh/personal/cpp/veq-engine/benchmarks
# Build directory: /home/nikaoh/workspace/github.com/oakinh/personal/cpp/veq-engine/build/libcxx/benchmarks
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[veq_bench_smoke]=] "/home/nikaoh/workspace/github.com/oakinh/personal/cpp/veq-engine/build/libcxx/benchmarks/veq_bench" "--benchmark_min_time=0.01" "--benchmark_filter=.")
set_tests_properties([=[veq_bench_smoke]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/nikaoh/workspace/github.com/oakinh/personal/cpp/veq-engine/benchmarks/CMakeLists.txt;43;add_test;/home/nikaoh/workspace/github.com/oakinh/personal/cpp/veq-engine/benchmarks/CMakeLists.txt;0;")
subdirs("../_deps/googlebenchmark-build")
