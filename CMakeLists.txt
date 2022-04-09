cmake_minimum_required(VERSION 3.21)
project(labirint)

set(CMAKE_CXX_STANDARD 23)

add_executable(labirint main.cpp)


target_link_libraries(${PROJECT_NAME} -lncurses)