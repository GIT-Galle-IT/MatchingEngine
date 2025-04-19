cmake_minimum_required(VERSION 3.22)
project(MatchingEngine)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# libs
add_subdirectory(gbase)

# Source
add_subdirectory(src)

# benchmark
add_subdirectory(tests)

# tests
add_subdirectory(benchmark)

# demos
add_subdirectory(demos)

