cmake_minimum_required(VERSION 3.8)

# Proyecto
project("Practica2_1_2")

# Habilite Recarga activa para los compiladores de MSVC si se admiten.
if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Añadir las opciones de compilación
if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(-O2)
endif()

# Añadir el ejecutable
add_executable(Practica2_1_2 
    "Practica2_1_2.cpp"
    "Original/Crystalline.cpp"
    "Original/Metallic.cpp"
    "Original/random.cpp"
    "Original/Scene.cpp"
    "Original/Sphere.cpp"
    "Original/utils.cpp"
)

# Estándar de C++
if(CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET Practica2_1_2 PROPERTY CXX_STANDARD 20)
endif()

# Encontrar y enlazar MPI
find_package(MPI REQUIRED)
include_directories(SYSTEM ${MPI_C_INCLUDE_DIRS})
target_link_libraries(Practica2_1_2 ${MPI_C_LIBRARIES})

# Flags del compilador
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")

# TODO: Agregue pruebas y destinos de instalación si es necesario.
