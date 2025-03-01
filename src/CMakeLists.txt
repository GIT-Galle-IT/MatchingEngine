include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

find_package(Threads REQUIRED)

file(GLOB_RECURSE SOURCES_FILES
    ${CMAKE_SOURCE_DIR}/src/matching/exchange/data/*.cpp
    ${CMAKE_SOURCE_DIR}/src/matching/exchange/impl/*.cpp
    ${CMAKE_SOURCE_DIR}/src/matching/exchange/*.cpp
    ${CMAKE_SOURCE_DIR}/src/matching/gbase/*.cpp
    ${CMAKE_SOURCE_DIR}/src/matching/utils/*.cpp
)

file(GLOB_RECURSE HEADERS_FILES 
    ${CMAKE_SOURCE_DIR}/src/matching/exchange/data/*.h
    ${CMAKE_SOURCE_DIR}/src/matching/exchange/impl/*.h
    ${CMAKE_SOURCE_DIR}/src/matching/exchange/*.h
    ${CMAKE_SOURCE_DIR}/src/matching/gbase/*.h
    ${CMAKE_SOURCE_DIR}/src/matching/utils/*.h
)


add_executable(
    matching_engine
    matching/MEServer.cpp
    ${SOURCES_FILES}
    ${HEADERS_FILES}
)

target_include_directories(matching_engine PRIVATE ${PROJECT_SOURCE_DIR}/src/include ${PROJECT_SOURCE_DIR}/src/utils)
target_link_libraries(matching_engine PRIVATE Threads::Threads)
