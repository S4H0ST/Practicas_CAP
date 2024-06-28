# CMakeList.txt: proyecto de CMake para Practica1_3, incluya el origen y defina
# la lógica específica del proyecto aquí.
#
cmake_minimum_required (VERSION 3.10)

# Habilite Recarga activa para los compiladores de MSVC si se admiten.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
endif()

project ("Practica1_3")

set(CMAKE_C_STANDARD 11)
if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
  add_compile_options(-O2)
endif()

# Configuración condicional para CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
if(MSVC)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "EditAndContinue")en 
endif()

# Agregue un origen al ejecutable de este proyecto.
add_executable (Practica1_3 "Practica1_3.cpp" "Practica1_3.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Practica1_3 PROPERTY CXX_STANDARD 20)
endif()
find_package(MPI)
include_directories(SYSTEM ${MPI_C_INCLUDE_DIRS})

target_link_libraries(Practica1_3  ${MPI_C_LIBRARIES})
# TODO: Agregue pruebas y destinos de instalación si es necesario.


# TODO: Agregue pruebas y destinos de instalación si es necesario.
