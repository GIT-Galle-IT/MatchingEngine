cmake_minimum_required(VERSION 3.20)
project(MatchingEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(DEBUG "Enable or disable DEBUG" ON)

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
