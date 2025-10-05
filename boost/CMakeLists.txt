cmake_minimum_required(VERSION 3.20)
project(BoostExample)

set(BOOST_INCLUDE_LIBRARIES asio regex algorithm)
set(BOOST_ENABLE_CMAKE ON)
FetchContent_Declare(
  Boost
  URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.7z
  USES_TERMINAL_DOWNLOAD TRUE
  DOWNLOAD_NO_EXTRACT FALSE
)
FetchContent_MakeAvailable(Boost)

add_executable(my_app asio_poc.cpp)
target_include_directories(my_app PRIVATE ${Boost_INCLUDE_DIRS})