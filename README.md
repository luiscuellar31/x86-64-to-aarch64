# x86-64-to-aarch64

Prototipo de traductor estático de un subconjunto de ensamblador x86-64 textual a ensamblador AArch64 textual con fines didácticos y académicos.

Este proyecto forma parte de una tesina orientada al diseño e implementación de una herramienta limitada, reproducible y evaluable para estudiar la traducción estática entre arquitecturas de conjuntos de instrucciones (ISA).

---

## Objetivo

Diseñar e implementar un traductor determinista de código fuente a código fuente que reciba ensamblador x86-64 simplificado (sintaxis Intel / NASM) y genere código ensamblador AArch64 equivalente dentro de un subconjunto formalmente delimitado, proporcionando metadatos didácticos y diagnósticos estructurados.

---

## Arquitectura

La traducción se realiza mediante reglas explícitas de correspondencia arquitectónica a través del siguiente pipeline desacoplado:

```text
Código x86-64 textual
        ↓
Normalización (limpieza y formato canónico)
        ↓
Análisis léxico (tokenización semántica)
        ↓
Análisis sintáctico (AST lineal de instrucciones x86)
        ↓
Validación semántica (verificación de operandos, anchos y registros)
        ↓
Motor de traducción (aplicación de reglas deterministas x86 → AArch64)
        ↓
Representación intermedia destino (IR AArch64)
        ↓
Emisión textual y generación de mapeos pedagógicos
```

---

## Estado actual

El núcleo del traductor se encuentra desacoplado como la biblioteca estática `traductor_nucleo` (`libtraductor_nucleo.a`), consumida por la interfaz de línea de comandos (CLI), la interfaz gráfica opcional y las pruebas automatizadas.

La CLI permite traducir archivos, guardar el resultado y consultar ayuda, versión, explicaciones y representaciones intermedias. El análisis sintáctico exige comas entre operandos, expresiones completas y literales numéricos dentro del rango de `int64_t`.

La interfaz gráfica presenta la entrada x86-64 junto a la salida AArch64, con acciones para traducir, limpiar, cargar la entrada y guardar la salida. El panel inferior muestra las explicaciones y los diagnósticos del mismo núcleo.

El validador comprueba registros y accesos a memoria de 64 bits, bases locales `rbp`/`rsp`, usos permitidos de `rsp` y etiquetas con una única definición. Las restricciones se detallan en [SINTAXIS_ENTRADA.md](docs/SINTAXIS_ENTRADA.md).

Las pruebas verifican la emisión, los diagnósticos y los mapeos didácticos. Aún faltan comprobaciones generales de codificación de operandos, dependencias de flags y alineación, balance y distribución de la pila. La aceptación de un programa y el ensamblado de su salida no demuestran equivalencia funcional para cualquier entrada.

### Capacidades implementadas
- **Modelo de datos semántico:** Soporte de operandos tipados (registros, inmediatos numéricos de 64 bits, etiquetas y memoria local estructurada con base y desplazamiento).
- **Instrucciones soportadas:**
  - Movimiento de datos: `mov reg, reg`, `mov reg, imm`, carga desde stack `mov reg, [rbp - imm]` (`ldr`) y almacenamiento en stack `mov [rbp - imm], reg` (`str`)
  - Gestión de pila: `push reg`, `pop reg`
  - Aritmética básica: `add reg, reg`, `add reg, imm`, `sub reg, reg`, `sub reg, imm`
  - Operaciones lógicas: `and reg, reg`, `and reg, imm`, `or reg, reg`, `or reg, imm`, `xor reg, reg`, `xor reg, imm`
  - Comparación y control de flujo: `cmp reg, reg`, `cmp reg, imm`, `jmp etiqueta`, saltos condicionales con signo (`je`, `jne`, `jl`, `jle`, `jg`, `jge`), llamadas a subrutina `call etiqueta`, retorno `ret` y definiciones de etiquetas (`etiqueta:`)
- **Llamadas a subrutina y registro de enlace:** Traducción de `call etiqueta` a una secuencia que guarda `x30` (`lr`) en un bloque temporal de 16 bytes, ejecuta `bl` y restaura el enlace al volver. Sólo llamadas internas bajo la convención y las restricciones de [ALCANCE_ABI.md](docs/ALCANCE_ABI.md), con un mapeo didáctico para las tres instrucciones destino.
- **Gestión de pila, prólogo y epílogo:** Traducción de `push reg` y `pop reg`, con registro distinto de `rsp`, mediante `str` con pre-indexado (`[sp, #-16]!`) y `ldr` con post-indexado (`[sp], #16`). Estas reglas conservan una alineación previa de 16 bytes y permiten secuencias canónicas de marco de pila (`push rbp; mov rbp, rsp` y `mov rsp, rbp; pop rbp; ret`).
- **Memoria local controlada:** Mapeo de accesos a pila con base `rbp` a `x29` y con base `rsp` a `sp` mediante `ldr` y `str`, respetando el orden arquitectónico de operandos (registro primero en `str`) y rechazando direccionamientos complejos no permitidos.
- **Traducción de 2 a 3 operandos:** Mapeo automático de la semántica destructiva de x86-64 (`add rax, 5`) a la forma explícita de tres operandos en AArch64 (`add x0, x0, #5`).
- **Diagnósticos estructurados:** Detección de errores con número de línea, códigos tipificados (`E001`, `E014`, `E021`, `E022`, `E023`, `E024`, `E030`, `E040`, `E041`) y mensajes descriptivos en español. `E041` identifica una etiqueta definida más de una vez.
- **Mapeos didácticos:** Cada instrucción traducida vincula su línea de origen, regla aplicada y explicación técnica del mapeo arquitectónico.
- **Interfaz de línea de comandos (CLI) estable:** Despachador completo con opciones `--help`, `--version`, `--explain`, `--dump-ir` y salida a archivo mediante `-o <archivo>` o redirección estándar.

---

## Estructura

```text
x86-64-to-aarch64/
├── CMakeLists.txt          # Configuración de compilación y definición de objetivos
├── README.md               # Descripción general y guía de inicio rápido
├── ARCHITECTURE.md         # Documento de arquitectura interna del sistema
├── docs/                   # Especificaciones técnicas y delimitación académica
│   ├── SINTAXIS_ENTRADA.md
│   ├── INSTRUCCIONES_SOPORTADAS.md
│   ├── POLITICA_REGISTROS.md
│   ├── POLITICA_FLAGS.md
│   └── ALCANCE_ABI.md
├── include/                # Encabezados públicos de la biblioteca traductor_nucleo
│   ├── instruccion.hpp
│   ├── normalizador.hpp
│   ├── analizador_lexico.hpp
│   ├── analizador_sintactico.hpp
│   ├── ir_aarch64.hpp
│   ├── tabla_registros.hpp
│   ├── validador.hpp
│   ├── traductor.hpp
│   ├── diagnostico.hpp
│   ├── resultado_traduccion.hpp
│   └── opciones_cli.hpp
├── src/                    # Implementación del núcleo y sus interfaces
│   ├── main.cpp            # Punto de entrada de la herramienta de línea de comandos
│   ├── gui/                # Ventana y punto de entrada de la interfaz gráfica opcional
│   └── ...                 # Módulos de implementación en C++17
├── tests/                  # Suites de pruebas unitarias y de integración (CTest)
└── examples/               # Corpus de ejemplos en ensamblador
    ├── validos/            # Programas x86-64 válidos
    └── invalidos/          # Casos de prueba para verificar diagnósticos de error
```

---

## Compilación y ejecución

### Requisitos previos
- Compilador de C++ con soporte para **C++17** (ej. Clang, GCC o Apple Clang).
- **CMake** 3.22 o superior.

### Construcción del proyecto
```bash
# Configurar el directorio de construcción
cmake -B build

# Compilar la biblioteca y todos los ejecutables
cmake --build build
```

### Ejecución de las pruebas
```bash
# Ejecutar todas las suites de prueba mediante CTest
ctest --test-dir build --output-on-failure
```

Las pruebas de traducción comparan la salida AArch64 completa con textos esperados escritos explícitamente. Cubren las formas de instrucciones implementadas, el mapeo de registros, las bases y desplazamientos de memoria local, las etiquetas y la preservación del enlace en llamadas internas.

Las pruebas de rechazo cubren AVX, SSE, x87, sintaxis AT&T, tamaños no soportados, direccionamientos complejos, mnemónicos desconocidos y formas de llamada excluidas. Comprueban el código y la línea del diagnóstico, la ausencia de salida parcial y la conservación de un archivo de salida previo al procesar el corpus inválido desde la CLI.

Si el compilador configurado es Clang con interfaz GNU y backend AArch64 disponible, CTest traduce y ensambla cada archivo `.s` de `examples/validos/` para `aarch64-unknown-linux-gnu`. Cada ejemplo tiene una prueba independiente con tiempo límite, que falla ante errores de traducción o del ensamblador. Se generan archivos objeto sin enlazar ni ejecutar código; no hacen falta bibliotecas del destino. CMake informa si estas pruebas no pueden registrarse con el compilador disponible.

El ensamblado comprueba la sintaxis y codificación de los ejemplos del corpus. El núcleo todavía puede aceptar otros operandos no codificables, y estas pruebas no demuestran equivalencia funcional general.

En macOS arm64 se añade una prueba nativa de retornos sucesivos y llamadas anidadas, con tiempo límite. Usa el compilador configurado y el ejemplo `09_llamadas_anidadas.s`; no requiere emulador ni dependencias nuevas.

En ese host, si también se pueden compilar y ejecutar binarios x86-64 mediante Rosetta, se comparan las funciones de `tests/funcionales/` con sus traducciones AArch64. Un arnés común utiliza 16 pares de enteros de 64 bits, incluidos extremos, y repite cada evaluación tres veces. Los casos cubren aritmética, lógica, comparación con signo, un bucle acotado, memoria local y llamadas internas. La adaptación de los dos argumentos pertenece al arnés de pruebas; no amplía la ABI del traductor. CMake comprueba la disponibilidad y las ejecuciones tienen tiempo límite.

Estas comparaciones verifican los casos y entradas indicados; los controles generales de flags, distribución de pila y codificación de operandos siguen pendientes.

### Registro de métricas

```bash
cmake -S . -B build
cmake --build build --target metricas
```

Este objetivo construye los ejecutables necesarios, mide los programas del corpus y ejecuta las pruebas configuradas. Guarda `CORPUS.md`, `pruebas.xml` (JUnit) y `pruebas.log` dentro de `build/metricas/`. El informe del corpus cuenta instrucciones del AST y de la IR AArch64, excluyendo etiquetas, y conserva los diagnósticos con sus líneas. Si una clasificación del corpus cambia o falla una prueba, el objetivo termina con error. Consulta [METRICAS.md](docs/METRICAS.md) para las definiciones, la medición de referencia y las restricciones semánticas.

### Interfaz gráfica opcional

Requiere Qt 6 con el módulo Widgets. La construcción habitual del núcleo y la CLI continúa funcionando sin Qt. Para habilitar la ventana:

```bash
cmake -S . -B build-gui -DCMAKE_BUILD_TYPE=Debug -DCONSTRUIR_GUI=ON \
  -DCMAKE_PREFIX_PATH="/ruta/al/directorio/de/Qt"
cmake --build build-gui
./build-gui/x86_64_to_aarch64_gui
```

`CMAKE_PREFIX_PATH` debe apuntar al directorio de la instalación que contiene `lib/cmake/Qt6/`; puede omitirse si CMake ya encuentra Qt. La construcción se verificó con Qt 6.11.2 y AppleClang 21 en macOS arm64. Qt debe ser compatible con el SDK instalado: en este entorno, Qt 6.8.3 falló al enlazar por una dependencia de AGL ausente del SDK.

La entrada admite edición y carga de archivos UTF-8. **Traducir** llama a `Traductor::traducir`; la salida y el panel inferior son de sólo lectura. **Limpiar** vacía los paneles. **Guardar** escribe únicamente la salida actual mediante `QSaveFile`, que reemplaza el archivo al completar la escritura. Si falla la lectura, se conserva la entrada anterior; si cambia la entrada o se rechaza una traducción, se borra el resultado anterior y se deshabilita Guardar para evitar guardar una salida desactualizada.

Cargar y Limpiar conservan el historial de deshacer del editor: se puede recuperar la entrada anterior con Deshacer. La recuperación vuelve a requerir una traducción para generar y guardar la salida correspondiente.

La ventana muestra las restricciones conocidas de flags, pila e inmediatos. Produce ensamblador textual con el alcance descrito en [METRICAS.md](docs/METRICAS.md). Las explicaciones y los diagnósticos proceden del núcleo; la interfaz no contiene reglas de traducción.

Con `CONSTRUIR_GUI=ON`, CTest añade `TestVentanaTraductor`, que comprueba los paneles, traducción, rechazo, invalidación de resultados, carga y guardado. Se ejecuta con `QT_QPA_PLATFORM=offscreen` y genera `ventana_traductor.png` en el directorio de construcción para revisar la distribución visual. Los diálogos nativos de selección de archivos requieren comprobación manual.

### Uso de la herramienta CLI

```bash
# Ayuda y opciones disponibles
./build/x86_64_to_aarch64 --help

# Versión de la herramienta
./build/x86_64_to_aarch64 --version

# Demostración interactiva interna (sin argumentos)
./build/x86_64_to_aarch64

# Traducción directa hacia la salida estándar
./build/x86_64_to_aarch64 examples/validos/01_mov_basico.s

# Traducción con guardado en archivo de salida (-o)
./build/x86_64_to_aarch64 examples/validos/01_mov_basico.s -o salida.s

# Traducción con explicaciones pedagógicas detalladas (--explain)
./build/x86_64_to_aarch64 examples/validos/08_llamadas_funciones.s --explain

# Volcado de la representación intermedia AST e IR (--dump-ir)
./build/x86_64_to_aarch64 examples/validos/08_llamadas_funciones.s --dump-ir
```

---

## Fuera del alcance inicial

Para mantener el proyecto delimitado y defendible académicamente, se excluyen deliberadamente:
- Traducción binaria dinámica o interpretación de binarios ELF/Mach-O.
- Operaciones con registros de 8, 16 o 32 bits (prevención de complejidad por extensión de ceros y escrituras parciales).
- Instrucciones vectoriales o de coma flotante (SSE, AVX, NEON).
- Llamadas al sistema operativo complejas y enlazado dinámico.
- Inferencia no determinista o delegación de traducción a modelos de lenguaje (LLM).

---

## Licencia

Este proyecto se distribuye bajo la licencia Apache License 2.0. Consulta el archivo `LICENSE` para más información.
