cmake_minimum_required(VERSION 3.10)
project(BoostExample)

find_package(Boost REQUIRED)

add_executable(my_app asio_poc.cpp)
target_include_directories(my_app PRIVATE ${Boost_INCLUDE_DIRS})