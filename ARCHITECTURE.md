# Arquitectura del Proyecto

Este documento describe la estructura del traductor `x86-64-to-aarch64`, cómo se procesa el código ensamblador paso a paso y las convenciones para colaborar en el repositorio.

---

## 1. Visión General

El proyecto es un traductor estático de código fuente a código fuente desarrollado en C++17. Su objetivo es tomar un subconjunto simplificado de ensamblador x86-64 (sintaxis Intel/NASM) y generar código equivalente en ensamblador AArch64 mediante reglas deterministas de correspondencia arquitectónica.

---

## 2. Flujo de Traducción

El procesamiento de una instrucción sigue una secuencia lineal y desacoplada:

1. **Normalización:** Limpia espacios en blanco, ignora comentarios y estandariza mayúsculas/minúsculas conservando los números de línea originales.
2. **Análisis léxico y sintáctico:** Identifica mnemónicos, operandos (registros, inmediatos, etiquetas) y construye las estructuras en memoria (`Instruccion`, `Operando`).
3. **Validación:** Verifica que la combinación y cantidad de operandos sean válidas dentro del subconjunto soportado.
4. **Motor de traducción:** Aplica el mapeo de registros y la transformación semántica correspondiente de x86-64 hacia AArch64.
5. **Emisión de código:** Genera las líneas finales en ensamblador AArch64 junto con diagnósticos o notas explicativas del cambio.

---

## 3. Estructura del Repositorio

A medida que el proyecto avance, los archivos se organizan de la siguiente manera:

- `src/`: Código fuente del núcleo del traductor y punto de entrada.
- `include/`: Encabezados públicos y definiciones de estructuras de datos.
- `docs/`: Especificaciones técnicas (sintaxis permitida, tablas de registros y reglas por instrucción).
- `examples/`: Archivos `.s` o `.asm` de prueba, divididos en casos válidos e inválidos.
- `tests/`: Pruebas automatizadas para validar el comportamiento del pipeline.

---

## 4. Convenciones de Código

Para mantener el código simple, uniforme y fácil de seguir:

- **Legibilidad ante todo:** Priorizamos código claro, directo y modular. Si una función requiere varias líneas explícitas en lugar de una expresión condensada o un truco sintáctico difícil de leer, preferimos las líneas explícitas.
- **Idioma:** Todo el código interno, variables, comentarios y nombres de archivos se escriben en español, manteniendo términos técnicos universales (`opcode`, `register`, `immediate`, `stack`).
- **Nomenclatura:**
  - Archivos de código: `snake_case` (ej. `analizador_lexico.cpp`, `instruccion.hpp`).
  - Documentos (`.md`): Mayúsculas / `SCREAMING_SNAKE_CASE` (ej. `README.md`, `ARCHITECTURE.md`, `SINTAXIS_ENTRADA.md`).
  - Variables y funciones: `camelCase` (ej. `registroDestino`, `analizarLinea`).
  - Clases, estructuras y enums: `PascalCase` (ej. `Instruccion`, `TipoOperando`).
  - Constantes: `SCREAMING_SNAKE_CASE` (ej. `MAX_OPERANDOS`).

---

## 5. Compilación

El proyecto utiliza CMake (versión mínima 3.22):

```bash
# Configuración inicial
cmake -S . -B build

# Compilar
cmake --build build

# Ejecutar
./build/x86_64_to_aarch64
```
