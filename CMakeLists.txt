cmake_minimum_required(VERSION 3.10)
project(SMS_general)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_C_COMPILER "clang")

add_executable(SMS_general main.cpp)
include_directories(include)
target_link_libraries(SMS_general pthread)

