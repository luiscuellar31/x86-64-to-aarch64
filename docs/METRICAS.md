# Métricas y límites de la validación

Las métricas describen el subconjunto implementado y los casos comprobados. Que las pruebas pasen no demuestra equivalencia funcional para cualquier programa que el traductor acepte.

## Cómo reproducir la medición

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target metricas
```

El objetivo `metricas` construye los ejecutables necesarios y produce estos archivos en `build/metricas/`:

| Archivo | Contenido |
| --- | --- |
| `CORPUS.md` | Una fila por programa: clasificación esperada y observada, instrucciones de origen y destino, códigos de diagnóstico y líneas originales. |
| `pruebas.xml` | Resultados de CTest en formato JUnit: casos registrados, aprobados, fallidos y omitidos. |
| `pruebas.log` | Salida de CTest para revisar cada resultado y los mensajes de fallo. |

El informe del corpus usa la biblioteca del traductor. Lee los archivos `.s` de `examples/validos/`, `examples/invalidos/` y `tests/funcionales/`, ordenados por nombre dentro de cada carpeta. Si una clasificación cambia, una carpeta no contiene programas, falla la lectura o falla una prueba, el objetivo termina con error. Los informes generados permanecen en el directorio de construcción, fuera de Git.

Las pruebas de ensamblado y ejecución se registran según las herramientas y el host disponibles, como explica el [README](../README.md#ejecución-de-las-pruebas). Los resultados deben compararse junto con esa configuración: un 100 % con menos pruebas registradas no implica haber realizado los mismos controles.

## Definiciones

- **Mnemónicos y formas:** se cuentan los mnemónicos distintos y las filas de la [matriz de instrucciones](INSTRUCCIONES_SOPORTADAS.md). Una forma es una combinación documentada de instrucción y tipos de operandos; no se cuentan por separado cada registro, inmediato o desplazamiento. Las etiquetas no son instrucciones. Este conteo expresa reglas de traducción implementadas, con las restricciones de la matriz.
- **Pruebas:** se cuenta cada caso registrado en CTest. Una suite con varios `assert` cuenta como una prueba, y una comparación funcional con múltiples entradas también cuenta como una prueba.
- **Porcentaje aprobado:** pruebas aprobadas divididas entre pruebas registradas, multiplicado por 100. Una prueba omitida no cuenta como aprobada. El XML conserva los resultados individuales para revisar esa distinción.
- **Errores rechazados:** se cuentan los archivos del corpus inválido rechazados. Las suites del traductor y de la CLI comprueban además sus diagnósticos, líneas originales y ausencia de salida parcial; no es una cuenta de todas las entradas inválidas posibles.
- **Instrucciones x86:** entradas de instrucción del AST, excluyendo etiquetas puras. En programas rechazados pueden incluir instrucciones incompletas o desconocidas; no representan instrucciones ejecutables verificadas.
- **Instrucciones AArch64:** entradas de la IR destino, excluyendo etiquetas puras. Una etiqueta junto a una instrucción no cambia el conteo. Cada `call` genera tres entradas. Se cuentan instrucciones emitidas, no ejecuciones de un bucle ni instrucciones máquina después del ensamblado; tampoco se incluyen los wrappers del arnés funcional.

## Medición de referencia

Medición del 13 de octubre de 2026, con AppleClang 21 en macOS arm64 y ejecución x86-64 mediante Rosetta disponible. Las reglas del núcleo y la matriz corresponden a `6f0464c`; la medición añade únicamente herramientas de registro y documentación.

| Indicador | Resultado |
| --- | ---: |
| Mnemónicos con reglas implementadas | 18 |
| Formas documentadas | 27 |
| Programas medidos | 31 |
| Ejemplos válidos aceptados | 9 de 9 |
| Funciones del experimento aceptadas | 6 de 6 |
| Archivos inválidos rechazados | 16 de 16 |
| Pruebas CTest aprobadas en Debug | 25 de 25 (100 %) |
| Pruebas CTest aprobadas en Release con UBSan para el núcleo | 25 de 25 (100 %) |
| Pruebas fallidas u omitidas en cada configuración | 0 |

En cada configuración se ejecutaron nueve suites del núcleo y la CLI, nueve controles de ensamblado, una prueba nativa de llamadas y seis comparaciones funcionales. Estas últimas usan 16 pares de entradas, repetidos tres veces por función: 288 resultados comparados por configuración. Sus ejecutables auxiliares se compilan por separado para cada arquitectura; UBSan se aplica a la construcción del núcleo y sus suites.

Los informes del corpus de Debug y Release coinciden exactamente. Los resultados JUnit y los logs completos quedan en los respectivos directorios de construcción; esta tabla es una referencia fechada y debe actualizarse si cambian el corpus, la matriz o las pruebas.

### Instrucciones emitidas por programa aceptado

| Carpeta | Programa | x86 (AST) | AArch64 (IR) |
| --- | --- | ---: | ---: |
| `examples/validos` | `01_mov_basico.s` | 3 | 3 |
| `examples/validos` | `02_aritmetica_basica.s` | 4 | 4 |
| `examples/validos` | `03_programa_simple.s` | 5 | 5 |
| `examples/validos` | `04_operaciones_logicas.s` | 6 | 6 |
| `examples/validos` | `05_control_flujo.s` | 7 | 7 |
| `examples/validos` | `06_memoria_local.s` | 7 | 7 |
| `examples/validos` | `07_stack_prologo_epilogo.s` | 9 | 9 |
| `examples/validos` | `08_llamadas_funciones.s` | 7 | 9 |
| `examples/validos` | `09_llamadas_anidadas.s` | 14 | 20 |
| `tests/funcionales` | `aritmetica.s` | 4 | 4 |
| `tests/funcionales` | `bucle.s` | 7 | 7 |
| `tests/funcionales` | `comparacion.s` | 9 | 9 |
| `tests/funcionales` | `llamadas.s` | 9 | 13 |
| `tests/funcionales` | `logica.s` | 5 | 5 |
| `tests/funcionales` | `memoria_local.s` | 11 | 11 |

Los 16 programas rechazados generan cero instrucciones AArch64. Su conteo de AST y el diagnóstico individual se conservan en `CORPUS.md`.

### Rechazos comprobados en el corpus

Cada archivo inválido contiene un error esperado. Las pruebas comprueban su código y línea, el retorno de error de la CLI y la conservación de un archivo de salida previo.

| Código | Archivos | Casos representados |
| --- | ---: | --- |
| `E001` | 4 | AVX, SSE, x87 y mnemónico desconocido. |
| `E014` | 3 | Forma de memoria inválida, base ajena a `rbp`/`rsp` y uso excluido de `rsp` como dato. |
| `E021` | 4 | Registros de 8, 16 y 32 bits y operando inválido de stack. |
| `E022` | 1 | Demasiados operandos. |
| `E030` | 2 | Sintaxis AT&T y direccionamiento complejo. |
| `E040` | 1 | Llamada a una etiqueta no definida. |
| `E041` | 1 | Etiqueta duplicada. |

La suite del traductor comprueba también 28 entradas rechazadas fuera de estos archivos, incluidas AVX y llamadas indirectas. Ese conjunto tiene categorías en común con el corpus; no se suma para afirmar que se cubren 44 categorías distintas.

## Casos fuera del soporte y restricciones semánticas

| Caso | Comportamiento o restricción actual |
| --- | --- |
| Subregistros de 8, 16 y 32 bits | Se rechazan; el subconjunto es entero de 64 bits. |
| SIMD, punto flotante, instrucciones fuera de la matriz y directivas | No tienen reglas de traducción y se rechazan. |
| Direccionamiento con índice/escala, relativo a `rip` o absoluto | Se rechaza. La memoria admitida usa `rbp` o `rsp` con desplazamiento simple. |
| Llamadas indirectas o a funciones externas | Quedan fuera del soporte. Las llamadas admitidas apuntan a etiquetas del mismo programa. |
| Flags de aritmética y lógica | `add`, `sub`, `and`, `or` y `xor` no actualizan NZCV en la salida. El validador aún no controla las dependencias de flags de todos los saltos; la [política de flags](POLITICA_FLAGS.md) detalla la restricción. |
| Diseño, balance y alineación de pila | `push`/`pop` usan bloques de 16 bytes frente a 8 en x86-64. No se validan globalmente la equivalencia de direcciones, la reserva, el balance ni la alineación. Los programas deben respetar las restricciones del [alcance ABI](ALCANCE_ABI.md). |
| Codificación de inmediatos y desplazamientos | El núcleo todavía acepta algunos valores que no pueden ensamblarse con la regla emitida, como `add rax, 5000`, `and rax, 0` o `mov rax, [rbp - 257]`. El control de ensamblado detecta el problema si aparece en sus casos; la aceptación de otro programa no garantiza su codificación. |
| Texto de literales | La emisión conserva el literal original; aún falta comprobar en general que su interpretación en el ensamblador destino coincida con el valor del AST. |
| ABI pública y argumentos en pila | El mapeo de registros es una convención interna. No adapta interfaces públicas ni argumentos en pila; el experimento funcional adapta únicamente dos argumentos mediante un wrapper de pruebas. |

Los resultados funcionales acreditan los programas y entradas del experimento. Los controles pendientes de flags, pila y codificación deben resolverse o delimitarse antes de presentar garantías más amplias.
