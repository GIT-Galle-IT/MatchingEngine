include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

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
