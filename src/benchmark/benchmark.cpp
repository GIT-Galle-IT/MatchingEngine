#include <benchmark/benchmark.h>
#include "../matching/MatchingEngine.hpp"

static void BM_MatchingAlgorithmPriceTime(benchmark::State& state) {
  std::map<std::string, int> symbol_to_index_map{{"AAPL" , 1}};
  long instrumentIndex(1);
  MatchingEngine me{{instrumentIndex}};
  for (auto _ : state)
  {
    state.PauseTiming();
    auto order = orderbook::Order{state.range(0), state.range(1), 
          (int)state.range(2), state.range(3), static_cast<orderbook::Order::OrderType>(state.range(4))};
    state.ResumeTiming();
    me.matchOrder(order);
  }
}
// Register the function as a benchmark
BENCHMARK(BM_MatchingAlgorithmPriceTime)
                  //  time_stamp, quantity, price,  instrument, order_type
                ->Args({401, 17, 10, 1, static_cast<int>(orderbook::Order::OrderType::SELL)});

static void BM_MatchingAlgorithmPriceTimePartialMatchedandFullMtaches(benchmark::State& state) {
  std::map<std::string, int> symbol_to_index_map{{"AAPL" , 1}};
  long instrumentIndex(1);
  MatchingEngine me{{instrumentIndex}};
  for (auto _ : state)
  {
    state.PauseTiming();
    auto order_ = orderbook::Order{400, 17, 10, 1, orderbook::Order::OrderType::BUY};
    me.matchOrder(order_);
    auto order = orderbook::Order{state.range(0), state.range(1), 
          (int)state.range(2), state.range(3), static_cast<orderbook::Order::OrderType>(state.range(4))};
    state.ResumeTiming();
    me.matchOrder(order);
  }
}
// Register the function as a benchmark
BENCHMARK(BM_MatchingAlgorithmPriceTimePartialMatchedandFullMtaches)
                  //  time_stamp, quantity, price,  instrument, order_type
                ->Args({401, 1, 10, 1, static_cast<int>(orderbook::Order::OrderType::SELL)})
                ->Args({402, 2, 10, 1, static_cast<int>(orderbook::Order::OrderType::SELL)})
                ->Args({403, 3, 10, 1, static_cast<int>(orderbook::Order::OrderType::SELL)})
                ->Args({404, 4, 10, 1, static_cast<int>(orderbook::Order::OrderType::SELL)})
                ->Args({408, 17, 10, 1, static_cast<int>(orderbook::Order::OrderType::SELL)}); // perfect match

BENCHMARK_MAIN();