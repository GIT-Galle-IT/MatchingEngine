FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        eddb0241389718a23a42db6af5f0164b6e0139af # release-1.9.1
)
FetchContent_MakeAvailable(benchmark)

add_executable(
    benchmark_me
    benchmark.cpp
    ${CMAKE_SOURCE_DIR}/tests/Setup.hpp
)

target_link_libraries(benchmark_me benchmark::benchmark)