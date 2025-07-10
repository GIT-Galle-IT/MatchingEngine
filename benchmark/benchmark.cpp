#include <benchmark/benchmark.h>
// #include "../matching/MatchingEngine.hpp"

static void BM_MatchingAlgorithmPriceTime(benchmark::State& state) {
  // MatchingEngine me;
  // for (auto _ : state)
  // {
    //  me.matchOrder();
  // }
}

// Register the function as a benchmark
BENCHMARK(BM_MatchingAlgorithmPriceTime);
BENCHMARK_MAIN();