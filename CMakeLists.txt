cmake_minimum_required(VERSION 2.8.4)
project(sik2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_custom_target(sik2 COMMAND make -C ${sik2_SOURCE_DIR})
