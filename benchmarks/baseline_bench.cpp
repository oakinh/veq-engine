#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <numeric>
#include <vector>

namespace veq::bench {

constexpr std::size_t batch_size = 1024;

// -----------------------------------------------------------------------------
// Baseline: sequential scan over one int64 column
// -----------------------------------------------------------------------------
//
// Purpose:
// - Establish raw column scan cost.
// - This is your "how fast can I walk contiguous memory?" baseline.
//
// Expected behavior:
// - Very cache-friendly.
// - Mostly memory bandwidth / tight-loop limited for large inputs.
// - Branch-free inner loop.
//
static void BM_Baseline_ColumnScanSum(benchmark::State& state) {
    const auto row_count = static_cast<std::size_t>(state.range(0));

    std::vector<std::uint64_t> column(row_count);
    std::iota(column.begin(), column.end(), 0);

    for ([[maybe_unused]] auto _ : state) {
        std::uint64_t sum = 0;

        for (std::size_t i = 0; i < column.size(); ++i) {
            sum += column[i];
        }

        benchmark::DoNotOptimize(sum);
    }

    state.SetItemsProcessed(static_cast<std::int64_t>(state.iterations() * row_count));
    state.SetBytesProcessed(static_cast<std::int64_t>(
        state.iterations() * row_count * sizeof(std::uint64_t)
        ));
}

BENCHMARK(BM_Baseline_ColumnScanSum)
    ->RangeMultiplier(2)
    ->Range(batch_size, 1 << 24)
    ->Unit(benchmark::kMicrosecond);

// -----------------------------------------------------------------------------
// Baseline: filter into selection vector
// -----------------------------------------------------------------------------
//
// Purpose:
// - Establish cost of producing a selection vector.
// - This is the core primitive for your filter operator.
//
// Expected behavior:
// - Sequential input read.
// - Sequential-ish output write.
// - Branch behavior depends heavily on selectivity.
// - Around 50% selectivity is often worse for branch prediction.
//
static void BM_Baseline_FilterSelectionVector(benchmark::State& state) {
    const auto row_count = static_cast<std::size_t>(state.range(0));
    const auto threshold = static_cast<std::uint64_t>(state.range(1));

    std::vector<std::uint64_t> age(row_count);
    std::vector<std::uint32_t> selection;
    selection.reserve(row_count);

    for (std::size_t i = 0; i < row_count; ++i) {
        age[i] = static_cast<std::uint64_t>(i % 100);
    }

    for ([[maybe_unused]] auto _ : state) {
        selection.clear();

        for (std::size_t i = 0; i < age.size(); ++i) {
            if (age[i] >= threshold) {
                selection.push_back(static_cast<std::uint32_t>(i));
            }
        }

        benchmark::DoNotOptimize(selection.data());
        benchmark::DoNotOptimize(selection.size());
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(static_cast<std::int64_t>(state.iterations() * row_count));
    state.SetBytesProcessed(
        static_cast<std::int64_t>(state.iterations() * row_count * sizeof(std::uint64_t))
        );
}

// threshold 1  => high selectivity
// threshold 50 => ~50% selectivity
// threshold 99 => low selectivity
BENCHMARK(BM_Baseline_FilterSelectionVector)
    ->Args({1024, 1})
    ->Args({1024, 50})
    ->Args({1024, 99})
    ->Args({1 << 16, 1})
    ->Args({1 << 16, 50})
    ->Args({1 << 16, 99})
    ->Args({1 << 24, 1})
    ->Args({1 << 24, 50})
    ->Args({1 << 24, 99})
    ->Unit(benchmark::kMicrosecond);

// -----------------------------------------------------------------------------
// Baseline: projection using selection vector
// -----------------------------------------------------------------------------
//
// Purpose:
// - Establish cost of gathering selected rows from one column into output.
// - This approximates projection after filter.
//
// Expected behavior:
// - Sequential output writes.
// - Input access pattern depends on selection vector.
// - Dense selection is cache-friendly.
// - Sparse/random selection later will be much worse.
//
static void BM_Baseline_ProjectWithSelectionVector(benchmark::State& state) {
    const auto row_count = static_cast<std::size_t>(state.range(0));
    const auto stride = static_cast<std::size_t>(state.range(1));

    std::vector<std::uint64_t> input(row_count);
    std::vector<std::uint32_t> selection;
    std::vector<std::uint64_t> output;

    std::iota(input.begin(), input.end(), 0);

    for (std::size_t i = 0; i < row_count; i += stride) {
        selection.push_back(static_cast<std::uint32_t>(i));
    }

    output.resize(selection.size());

    for ([[maybe_unused]] auto _ : state) {
        for (std::size_t i = 0; i < selection.size(); ++i) {
            output[i] = input[selection[i]];
        }

        benchmark::DoNotOptimize(output.data());
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(static_cast<std::int64_t>(state.iterations() * selection.size()));
    state.SetBytesProcessed(static_cast<std::int64_t>(
        state.iterations() * selection.size() *
        (sizeof(std::uint32_t) + sizeof(std::uint64_t) + sizeof(std::uint64_t))
    ));
}

// stride 1  => dense projection
// stride 4  => semi-dense
// stride 16 => sparse
BENCHMARK(BM_Baseline_ProjectWithSelectionVector)
    ->Args({1024, 1})
    ->Args({1024, 4})
    ->Args({1024, 16})
    ->Args({1 << 16, 1})
    ->Args({1 << 16, 4})
    ->Args({1 << 16, 16})
    ->Args({1 << 24, 1})
    ->Args({1 << 24, 4})
    ->Args({1 << 24, 16})
    ->Unit(benchmark::kMicrosecond);

} // namespace

BENCHMARK_MAIN();