cmake_minimum_required(VERSION 3.23)
project(dreamer)

set(CMAKE_CXX_STANDARD 20)

include_directories(./include)

add_subdirectory(./test)
add_subdirectory(./src)
