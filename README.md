# Practicas_CAP

En el siguiente repositorio se recoge todas las practicas realizadas sobre la asignatura de Computación de altas prestaciones.

**Indice**
--
-  [Practica 1 - MPI](./Practica1)
- [Practica 2 - MPI Frame/Slave](./Practica2/)
- [Practica 2.2 - MPI & OPENMP Patch/Slave](./Practica2_2/)

Para ejecutar hay que:

1. Compilar el proyecto y que se genere .exe
2. Dirigirse al directorio donde el ejecutable y ejecutar con mpi 

Ejemplo para [Practica 2 - MPI Frame/Slave](./Practica2/)
```
cd out/build
mpiexec -n 4 .\Practica2.exe
```
Imagenes generadas del ejemplo : [Imagenes](./Practica2/out/)

> [!NOTE]    
> 4 es el numero de procesos: 1 es el Maestro y 3 los hijos, se generará 3 imagenes.
> En cada carpeta hay un resultado.txt con la información de compilación
> En la carpeta ```out``` se almacena las imagenes generadas 

