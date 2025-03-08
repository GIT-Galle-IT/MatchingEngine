FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        c58e6d0710581e3a08d65c349664128a8d9a2461 # release-1.9.1
  FIND_PACKAGE_ARGS NAMES GTest
)
FetchContent_MakeAvailable(benchmark)

find_package(benchmark REQUIRED)

add_executable(
    benchmark_me
    benchmark.cpp
    ${CMAKE_SOURCE_DIR}/tests/Setup.hpp
)

target_link_libraries(benchmark_me benchmark::benchmark)