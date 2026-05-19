include(FetchContent)

# ------------------------------------------------------------------------------
# Google Benchmark
# ------------------------------------------------------------------------------
FetchContent_Declare(
        googlebenchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_TESTING     OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_LTO         OFF CACHE BOOL "" FORCE)

# Disable clang-tidy for third-party dependency.
set(_OLD_TIDY "${CMAKE_CXX_CLANG_TIDY}")
set(CMAKE_CXX_CLANG_TIDY "")

FetchContent_MakeAvailable(googlebenchmark)

set(CMAKE_CXX_CLANG_TIDY "${_OLD_TIDY}")

# ------------------------------------------------------------------------------
# VEQ benchmarks
# ------------------------------------------------------------------------------
add_executable(veq_bench
        baseline_bench.cpp
        filter_bench.cpp
        projection_bench.cpp
        scan_bench.cpp
)

target_link_libraries(veq_bench
        PRIVATE
        veq::veq
        benchmark::benchmark
)

if(CMAKE_CXX_CLANG_TIDY)
    set_property(TARGET veq_bench PROPERTY CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY}")
endif()

# Smoke test only. This verifies the benchmark binary runs.
# It does not measure meaningful performance.
if(BUILD_TESTING)
    add_test(NAME veq_bench_smoke
            COMMAND veq_bench --benchmark_min_time=0.01 --benchmark_filter=.)
endif()