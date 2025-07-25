
file(GLOB_RECURSE SOURCES_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/net/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

file(GLOB_RECURSE HEADERS_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/net/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

add_library(gbase STATIC ${HEADERS_FILES} ${SOURCES_FILES})
if(DEBUG_LOG)
    target_compile_definitions(gbase PRIVATE DEBUG_LOG)
endif()
target_include_directories(gbase PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
