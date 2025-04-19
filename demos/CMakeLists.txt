include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

find_package(Protobuf REQUIRED)

add_executable(
    server
    DemoServer.cpp
)

add_executable(
    client
    DemoClient.cpp
)

target_link_libraries(server PRIVATE gbase ${Protobuf_LIBRARIES})
target_link_libraries(client PRIVATE gbase ${Protobuf_LIBRARIES})
