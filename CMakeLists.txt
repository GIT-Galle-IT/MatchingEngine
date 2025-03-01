cmake_minimum_required(VERSION 3.22)
project(MatchingEngine)

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
    
find_package(Threads REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)
  
add_executable(
        me
        src/cli_demo.cpp
    )

enable_testing()

add_executable(
    run_tests
    src/matching/MatchingEngine.hpp
    src/matching/MatchingEngine.cpp
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
    benchmark_me
    src/matching/MatchingEngine.hpp
    src/matching/MatchingEngine.cpp
    src/benchmark/benchmark.cpp
    tests/Setup.hpp
    ${SOURCES_FILES}
    ${HEADERS_FILES}
)

target_link_libraries(benchmark_me benchmark::benchmark)

file(GLOB_RECURSE SOURCES_FILES
    ${CMAKE_SOURCE_DIR}/src/matching/exchange/data/*.cpp
    ${CMAKE_SOURCE_DIR}/src/matching/exchange/impl/*.cpp
    ${CMAKE_SOURCE_DIR}/src/matching/exchang/*.cpp
    ${CMAKE_SOURCE_DIR}/src/matching/gbase/*.cpp
    ${CMAKE_SOURCE_DIR}/src/matching/refdata/data/*.cpp
    ${CMAKE_SOURCE_DIR}/src/matching/refdata/impl/*.cpp
    ${CMAKE_SOURCE_DIR}/src/matching/utils/*.cpp
)

file(GLOB_RECURSE HEADERS_FILES 
    ${CMAKE_SOURCE_DIR}/src/matching/exchange/data/*.h
    ${CMAKE_SOURCE_DIR}/src/matching/exchange/impl/*.h
    ${CMAKE_SOURCE_DIR}/src/matching/exchang/*.h
    ${CMAKE_SOURCE_DIR}/src/matching/refdata/data/*.h
    ${CMAKE_SOURCE_DIR}/src/matching/refdata/impl/*.h
    ${CMAKE_SOURCE_DIR}/src/matching/gbase/*.h
    ${CMAKE_SOURCE_DIR}/src/matching/utils/*.h
)

add_executable(
    matching_engine
    src/matching/MEServer.cpp
    ${SOURCES_FILES}
    ${HEADERS_FILES}
)

target_link_libraries(matching_engine PRIVATE Threads::Threads)