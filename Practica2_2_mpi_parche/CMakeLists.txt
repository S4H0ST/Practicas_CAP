# CMakeList.txt: proyecto de CMake para Practica2_2, incluya el origen y defina
# la lógica específica del proyecto aquí.
cmake_minimum_required (VERSION 3.8)
project ("Practica2_2")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/out/build")
# Agregue un origen al ejecutable de este proyecto.
if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
  add_compile_options(-O2)
endif()
add_executable (Practica2_2 "Practica2_2.cpp" 
        "Original/Crystalline.cpp"
        "Original/Metallic.cpp"
        "Original/random.cpp"
        "Original/Scene.cpp"
        "Original/Sphere.cpp"
        "Original/utils.cpp"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Practica2_2 PROPERTY CXX_STANDARD 20)
endif()

# TODO: Agregue pruebas y destinos de instalación si es necesario.
find_package(MPI)
include_directories(SYSTEM ${MPI_C_INCLUDE_DIRS})
target_link_libraries(Practica2_2 ${MPI_C_LIBRARIES})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")

