# CMakeList.txt: proyecto de CMake para Practica2, incluya el origen y defina
# la lógica específica del proyecto aquí.
#
cmake_minimum_required (VERSION 3.8)

project ("Practica2")
# Especificar la ubicación de salida para el ejecutable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/out/build")
if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(-O2)
endif()

# Agregue un origen al ejecutable de este proyecto.
add_executable (Practica2 "Practica2.cpp" 
        "Original/Crystalline.cpp"
        "Original/Metallic.cpp"
        "Original/random.cpp"
        "Original/Scene.cpp"
        "Original/Sphere.cpp"
        "Original/utils.cpp"
)
find_package(MPI)
include_directories(SYSTEM ${MPI_C_INCLUDE_DIRS})
target_link_libraries(Practica2 ${MPI_C_LIBRARIES})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")

# TODO: Agregue pruebas y destinos de instalación si es necesario.
