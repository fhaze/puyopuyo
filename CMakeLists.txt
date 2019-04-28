cmake_minimum_required(VERSION 3.13)
project(puyopuyo)

set(CMAKE_CXX_STANDARD 14)

find_package(Curses REQUIRED)
add_executable(puyopuyo main.cpp)
target_link_libraries(puyopuyo ${CURSES_LIBRARIES})