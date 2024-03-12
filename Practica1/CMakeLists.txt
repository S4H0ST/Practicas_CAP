cmake_minimum_required(VERSION 3.27)
project(Practica1_Cap C)

set(CMAKE_C_STANDARD 11)

add_executable(Practica1_Cap main.c)
find_package(MPI)
include_directories(SYSTEM ${MPI_C_INCLUDE_DIRS})

target_link_libraries(Practica1_Cap ${MPI_C_LIBRARIES})