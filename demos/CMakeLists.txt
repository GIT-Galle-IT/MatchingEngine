include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)



file(GLOB_RECURSE SOURCES_FILES
    ${CMAKE_SOURCE_DIR}/src/matching/gbase/net/*.cpp
)

# add_library(gbaselib STATIC ${PROJECT_SOURCE_DIR}/gbase/gbase_library.hpp)

add_executable(
    server
    DemoServer.cpp
)

add_executable(
    client
    DemoClient.cpp
)

target_link_libraries(server PRIVATE gbase)
target_link_libraries(client PRIVATE gbase)
