include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include_directories(${CMAKE_SOURCE_DIR})
set(SOURCES
  test/OrderBookTest.cpp)

enable_testing()

add_executable(
  run_tests
  OrderBookTest.cpp
  MatchingEngineTest.cpp
  Setup.hpp
  ByteBufferTest.cpp
  ProtocolTest.cpp
)
target_link_libraries(
  run_tests
  GTest::gtest_main
)
target_link_libraries(run_tests gbase)
include(GoogleTest)
gtest_discover_tests(run_tests)
