cmake_minimum_required(VERSION 3.22)
project(mini-ats)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(SOURCES
    test/OrderBookTest.cpp)

add_executable(
        ats
        src/matching/MatchingEngine.hpp
        src/matching/MatchingEngine.cpp
        src/orderbook/OrderBook.cpp
        src/orderbook/OrderBook.hpp
        src/cli_demo.cpp
    )

enable_testing()

add_executable(
    run_tests
    src/matching/MatchingEngine.hpp
    src/matching/MatchingEngine.cpp
    src/orderbook/OrderBook.cpp
    src/orderbook/OrderBook.hpp
    tests/OrderBookTest.cpp
    tests/MatchingEngineTest.cpp
    tests/Setup.hpp
)

target_link_libraries(
  run_tests
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(run_tests)

FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        c58e6d0710581e3a08d65c349664128a8d9a2461 # release-1.9.1
  FIND_PACKAGE_ARGS NAMES GTest
)
FetchContent_MakeAvailable(benchmark)

find_package(benchmark REQUIRED)

add_executable(
    benchmark_ats
    src/benchmark/benchmark.cpp
    src/matching/MatchingEngine.cpp
    src/matching/MatchingEngine.hpp
    src/orderbook/OrderBook.cpp
    src/orderbook/OrderBook.hpp
    tests/Setup.hpp
)

target_link_libraries(benchmark_ats benchmark::benchmark)