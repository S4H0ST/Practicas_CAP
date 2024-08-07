# Practicas_CAP

En el siguiente repositorio se recoge todas las practicas realizadas sobre la asignatura de Computación de altas prestaciones.

**Indice**
--
- [Practica 1 - MPI - Distribucion contigua de lineas](./Descipher_enigma/Practica1/)
- [Practica 1.2 - MPI - Distribucion por bloques de lineas](./Descipher_enigma/Practica1_2/)
- [Practica 1.3 - MPI - Distribución con Gather y Scatter](./Descipher_enigma/Practica1_3/)
- [Practica 2 - MPI Frame/proceso](./RayTracing/Practica2/)
- [Practica 2.1 - OpenMP Frame/proceso](./RayTracing/Practica2_1_openmp/)
- [Practica 2.1.2 - OpenMp + OpenMPI Frame/proceso](./RayTracing/Practica2_1_openmp_mpi/)
- [Practica 2.2 - MPI Parche/esclavo](./RayTracing/Practica2_2_mpi_parche/)

## Para ejecutar con MPI

1. Compilar el proyecto desde la raiz con el siguiente codigo que lo ejecuta en modo release donde en cmake esta configurado para usar la optimización -O2.
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
MSBuild Practica2.sln /p:Configuration=Release
```
>NOTA: en Practica2.sln añadir archivo *.sln que haya en la carpeta build del proyecto , no es siempre Practica2.sln.

2. Dirigirse al directorio donde el ejecutable y ejecutar con mpi 
```bash
cd Release
mpiexec -n 4 .\Practica2.exe
```
> [!Important]    
> Si pongo 4 es el numero de procesos: 1 es el Maestro y 3 los hijos, se generará 3 imagenes.
>
> En cada proyecto hay un **resultado.txt** con la información de compilación

## Para ejecutar con OPENMP

1. ejecutar el siguiente comando desde la raiz donde se ubique el proyecto.cpp, aplica la optimización -O3.

```bash
g++ -O3 -fopenmp Practica2_1.cpp Original/random.cpp Original/utils.cpp Original/Sphere.cpp Original/Scene.cpp  Original/Metallic.cpp Original/Crystalline.cpp -o Practica2_1
```
2. Ejecutar proyecto

```bash
.\main.exe
```

>Nota: para añadir un número de nodos, dentro del codigo encima de pragma parallel for , editar el numero de nodos.

Ejemplo :

```bash
omp_set_num_threads(100);
#pragma omp parallel
//...
```