cmake_minimum_required(VERSION 3.21)
project(labirint)

set(CMAKE_CXX_STANDARD 23)

add_executable(labirint main.cpp)

set(CMAKE_EXE_LINKER_FLAGS "-static")
