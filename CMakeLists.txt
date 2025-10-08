cmake_minimum_required(VERSION 3.20)
project(MatchingEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(DEBUG_LOG "Enable debug log" ON)

add_compile_options("$<$<CONFIG:Release>:-O3>" "$<$<CONFIG:Debug>:-g>" -Wall -Wextra -Wpedantic -save-temps)

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
