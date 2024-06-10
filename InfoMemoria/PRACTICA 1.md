PRACTICA 1

La diferencia principal entre el código original y el código modificado radica en la forma en que las tareas (líneas de texto cifrado) se distribuyen entre los procesos hijos. Vamos a desglosar esta diferencia:

Código Original
En el código original, las tareas se distribuyen de manera uniforme entre los procesos hijos, basándose en el cálculo de linesPerSlave:

```c
int linesPerSlave = nLines / (nProcesses - 1);
```
Cada proceso hijo recibe un bloque consecutivo de líneas, y el último proceso recibe cualquier línea adicional que no se pueda dividir de manera equitativa. El código para enviar y recibir líneas es relativamente simple y directo, pero no optimiza la distribución de carga entre los procesos si las líneas tienen diferentes tiempos de descifrado.

Código Modificado
En el código modificado, se introduce una distribución alternativa, donde cada proceso hijo aún recibe un bloque de líneas, pero se agrega una corrección para asegurar que las claves se formateen correctamente y se optimiza la distribución y procesamiento de las líneas. Los cambios se centran en:

Asignación de Líneas:

Se asegura que cada proceso reciba un número de líneas basado en su rango (rank), y el último proceso recibe las líneas restantes.
Este método mejora la distribución de la carga al considerar los rangos de los procesos.
Formateo de Claves:

Se formatea la clave para asegurar que tenga el mismo número de dígitos que el número de rotores (nRotors).
Esto mejora la precisión en la comparación de claves durante el descifrado.
Liberación de Memoria:

Se asegura la liberación de memoria para cada línea descifrada en los procesos hijos, previniendo fugas de memoria.
Diferencias en la Distribución de Tareas
Uniformidad vs. Dinamismo:

Código Original: Asigna bloques fijos de líneas a cada proceso. Esto puede ser menos eficiente si las líneas tienen diferentes tiempos de descifrado.
Código Modificado: Aunque también asigna bloques de líneas, la clave se gestiona mejor y se asegura la correcta liberación de memoria. La distribución se optimiza ligeramente para asegurar que cada proceso trabaje de manera eficiente con las líneas asignadas.
Flexibilidad:

Código Original: Menos flexible en la gestión de claves y puede tener problemas de fugas de memoria.
Código Modificado: Más flexible y robusto en la gestión de claves y la memoria, lo que mejora la eficiencia global del proceso de descifrado.
Ejemplo de Diferencias en la Ejecución:
Si nLines = 8 y nProcesses = 4:
Código Original: Cada proceso hijo recibiría 2 líneas (8 / 3 ≈ 2.67, truncado a 2).
Código Modificado: Similar asignación, pero con mejoras en la gestión de claves y memoria, asegurando una ejecución más fluida.
Conclusión
La principal diferencia radica en la eficiencia y robustez de la distribución y gestión de tareas. El código modificado introduce una mejor gestión de claves y liberación de memoria, lo que puede resultar en una ejecución más eficiente y menos propensa a errores en un entorno de procesamiento paralelo.