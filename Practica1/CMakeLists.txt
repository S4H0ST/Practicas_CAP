cmake_minimum_required(VERSION 3.27)
project(Practica1 C)

set(CMAKE_C_STANDARD 11)

add_executable(Practica1 main.c)

include_directories(SYSTEM ${MPI_C_INCLUDE_DIRS})

target_link_libraries(Practica1 ${MPI_C_LIBRARIES})