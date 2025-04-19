include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

find_package(Threads REQUIRED)
add_executable(
    matching_engine
    matching/MEServer.cpp
    ${SOURCES_FILES}
    ${HEADERS_FILES}
)

add_executable(
    trading_client
    tradingclientgw/TradingClient.cpp
    ${SOURCES_FILES}
    ${HEADERS_FILES}
)

include(FetchContent)

# FetchContent_Declare(ftxui
#   GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
#   GIT_TAG v6.0.2
# )

# FetchContent_GetProperties(ftxui)
# if(NOT ftxui_POPULATED)
#   FetchContent_Populate(ftxui)
#   add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
# endif()

# target_include_directories(matching_engine PRIVATE ${PROJECT_SOURCE_DIR}/src/include ${PROJECT_SOURCE_DIR}/src/utils)
target_link_libraries(matching_engine PRIVATE gbase)
# target_link_libraries(trading_client PRIVATE gbase)
target_link_libraries(matching_engine PRIVATE Threads::Threads)
# target_link_libraries(trading_client
#   PRIVATE ftxui::screen
#   PRIVATE ftxui::dom
#   PRIVATE ftxui::component  # Not needed for this example.
# )
