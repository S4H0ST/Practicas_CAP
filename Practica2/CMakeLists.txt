cmake_minimum_required(VERSION 3.27)
project(Practica2)

set(CMAKE_CXX_STANDARD 17)

add_executable(Practica2 main.cpp
        "Archivos originales/Crystalline.cpp"
        "Archivos originales/Metallic.cpp"
        "Archivos originales/random.cpp"
        "Archivos originales/Scene.cpp"
        "Archivos originales/Sphere.cpp"
        "Archivos originales/utils.cpp")
find_package(MPI)
include_directories(SYSTEM ${MPI_C_INCLUDE_DIRS})

target_link_libraries(Practica2 ${MPI_C_LIBRARIES})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")