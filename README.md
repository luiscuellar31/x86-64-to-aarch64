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

El núcleo del traductor se encuentra desacoplado como la biblioteca estática `traductor_nucleo` (`libtraductor_nucleo.a`), consumida tanto por la interfaz de línea de comandos (CLI) como por la suite de pruebas automatizadas.

El último milestone implementado es **3.6 (CLI estable)**. La consolidación de **7.1 (pruebas de parsing)** exige separadores y operandos completos y rechaza literales fuera de `int64_t`. Las llamadas internas del milestone 3.5 preservan el enlace de retorno. Todavía existen bloqueos semánticos en flags, stack y codificación de operandos antes de iniciar la GUI; el ensamblado de los ejemplos no acredita equivalencia funcional.

### Capacidades implementadas
- **Modelo de datos semántico:** Soporte de operandos tipados (registros, inmediatos numéricos de 64 bits, etiquetas y memoria local estructurada con base y desplazamiento).
- **Instrucciones soportadas:**
  - Movimiento de datos: `mov reg, reg`, `mov reg, imm`, carga desde stack `mov reg, [rbp - imm]` (`ldr`) y almacenamiento en stack `mov [rbp - imm], reg` (`str`)
  - Gestión de pila: `push reg`, `pop reg`
  - Aritmética básica: `add reg, reg`, `add reg, imm`, `sub reg, reg`, `sub reg, imm`
  - Operaciones lógicas: `and reg, reg`, `and reg, imm`, `or reg, reg`, `or reg, imm`, `xor reg, reg`, `xor reg, imm`
  - Comparación y control de flujo: `cmp reg, reg`, `cmp reg, imm`, `jmp etiqueta`, saltos condicionales con signo (`je`, `jne`, `jl`, `jle`, `jg`, `jge`), llamadas a subrutina `call etiqueta`, retorno `ret` y definiciones de etiquetas (`etiqueta:`)
- **Llamadas a subrutina y registro de enlace:** Traducción de `call etiqueta` a una secuencia que guarda `x30` (`lr`) en un bloque temporal de 16 bytes, ejecuta `bl` y restaura el enlace al volver. Sólo llamadas internas bajo la convención y las restricciones de [ALCANCE_ABI.md](docs/ALCANCE_ABI.md), con un mapeo didáctico para las tres instrucciones destino.
- **Gestión de pila, prólogo y epílogo:** Traducción de `push reg` y `pop reg` mediante `str` con pre-indexado (`[sp, #-16]!`) y `ldr` con post-indexado (`[sp], #16`), garantizando la estricta alineación a 16 bytes de la ABI AAPCS64 y permitiendo secuencias canónicas de marco de pila (`push rbp; mov rbp, rsp` y `mov rsp, rbp; pop rbp; ret`).
- **Memoria local controlada:** Mapeo de accesos a pila con base `rbp` a `x29` en AArch64 mediante `ldr` y `str`, respetando el orden arquitectónico de operandos (registro primero en `str`) y validando contra direccionamientos complejos no permitidos.
- **Traducción de 2 a 3 operandos:** Mapeo automático de la semántica destructiva de x86-64 (`add rax, 5`) a la forma explícita de tres operandos en AArch64 (`add x0, x0, #5`).
- **Diagnósticos estructurados:** Detección de errores con número de línea, códigos tipificados (`E001`, `E014`, `E021`, `E022`, `E023`, `E040`) y mensajes descriptivos en español.
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
├── src/                    # Implementación del núcleo y ejecutable CLI
│   ├── main.cpp            # Punto de entrada de la herramienta de línea de comandos
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

En macOS arm64 se añade una prueba nativa de retornos sucesivos y llamadas anidadas, con tiempo límite. Usa el compilador configurado y el ejemplo `09_llamadas_anidadas.s`; no requiere emulador ni dependencias nuevas.

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
