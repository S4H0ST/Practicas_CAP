cmake_minimum_required(VERSION 3.27)
project(Practica1_2 C)

set(CMAKE_C_STANDARD 11)
if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
  add_compile_options(-O2)
endif()

add_executable(Practica1_2  Practica1_2.c
)
find_package(MPI)
include_directories(SYSTEM ${MPI_C_INCLUDE_DIRS})

target_link_libraries(Practica1_2  ${MPI_C_LIBRARIES})