cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

set(BOOST_INCLUDE_LIBRARIES asio regex algorithm)
set(BOOST_ENABLE_CMAKE ON)
FetchContent_Declare(
  Boost
  URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.7z
  USES_TERMINAL_DOWNLOAD TRUE
  DOWNLOAD_NO_EXTRACT FALSE
)
FetchContent_MakeAvailable(Boost)

file(GLOB_RECURSE SOURCES_FILES
    ${CMAKE_SOURCE_DIR}/src/matching/gbase/net/*.cpp
)

add_executable(
    server
    DemoServer.cpp
)

add_executable(
    client
    DemoClient.cpp
)

add_executable(
    asio
    asio_poc.cpp
)

add_executable(
    async_server
    DemoAsyncServer.cpp
)

add_executable(
    async_client
    DemoAsyncClient.cpp
)

if(DEBUG_LOG)
    target_compile_definitions(async_server PRIVATE DEBUG_LOG)
    target_compile_definitions(server PRIVATE DEBUG_LOG)
    target_compile_definitions(client PRIVATE DEBUG_LOG)
    target_compile_definitions(async_client PRIVATE DEBUG_LOG)
    target_compile_definitions(asio PRIVATE DEBUG_LOG)
endif()

set_target_properties(asio PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(server PRIVATE gbase)
target_link_libraries(async_server PRIVATE gbase)
target_link_libraries(client PRIVATE gbase)
target_link_libraries(async_client PRIVATE gbase)
target_link_libraries(asio PRIVATE Boost::asio)
