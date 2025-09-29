set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

find_package(Boost 1.74.0 REQUIRED system)

IF (Boost_FOUND)
    MESSAGE(STATUS "Boost found. Version: ${Boost_VERSION}")
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
ELSE
    MESSAGE(STATUS "Boost not found. Please install Boost 1.74 or higher.")
ENDIF()

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

set_target_properties(asio PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(server PRIVATE gbase)
target_link_libraries(client PRIVATE gbase)
target_link_libraries(asio PRIVATE ${Boost_INCLUDE_DIRS})