
set(BOOST_INCLUDE_LIBRARIES asio regex algorithm)
set(BOOST_ENABLE_CMAKE ON)
# FetchContent_Declare(
#   Boost
#   URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.7z
#   USES_TERMINAL_DOWNLOAD TRUE
#   DOWNLOAD_NO_EXTRACT FALSE
# )
# FetchContent_MakeAvailable(Boost)

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
target_include_directories(gbase PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/net>
)