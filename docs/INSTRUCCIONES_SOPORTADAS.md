# Matriz de Instrucciones Soportadas

Este documento define la matriz técnica de correspondencia entre el subconjunto de x86-64 y AArch64.

El estado «Soportado» registra formas reconocidas y reglas de emisión implementadas. Todavía no garantiza equivalencia funcional para todos sus operandos o programas: faltan controles de codificación, flags y stack. Las llamadas internas preservan el enlace de retorno bajo las restricciones descritas en [ALCANCE_ABI.md](ALCANCE_ABI.md).

---

## 1. Alcance de Ancho de Palabra (64 bits)

El traductor opera **únicamente con registros y operaciones enteras de 64 bits**.
- **Soportados:** `rax`, `rbx`, `rcx`, `rdx`, `rsi`, `rdi`, `rbp`, `rsp`, `r8` a `r15`.
- **Excluidos inicialmente:** Subregistros de 32, 16 y 8 bits (`eax`, `ax`, `al`, `ah`, etc.) para evitar complejidad prematura con extensión de ceros (*zero-extension*) y escrituras parciales.
- `rsp` tiene un uso específico como stack pointer: las restricciones por forma se describen en [SINTAXIS_ENTRADA.md](SINTAXIS_ENTRADA.md#8-uso-limitado-de-rsp). La presencia de un registro en la lista no implica que sea válido en cualquier posición.

---

## 2. Formas de instrucciones reconocidas y traducción implementada

| Instrucción x86-64 | Forma | Estado | Afecta Flags x86 | Equivalente AArch64 | Notas y Explicación Didáctica |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `mov` | `reg, reg` | Soportado | No | `mov xD, xS` | Copia directa de registro a registro de 64 bits. |
| `mov` | `reg, imm` | Soportado | No | `mov xD, #imm` | Carga de constante inmediata en registro; destino distinto de `rsp`. |
| `mov` | `reg, [rbp - imm]` | Soportado | No | `ldr xD, [x29, #-imm]` | Carga desde memoria local del stack (`ldr`) con base `x29`. |
| `mov` | `[rbp - imm], reg` | Soportado | No | `str xS, [x29, #-imm]` | Almacenamiento en memoria local del stack (`str`) con base `x29`. En AArch64 `str`, el registro fuente va primero. |
| `add` | `reg, reg` | Soportado | Sí | `add xD, xD, xS` | x86 usa destino destructivo de 2 operandos; AArch64 expresa 3 operandos (`destino, fuente1, fuente2`). |
| `add` | `reg, imm` | Soportado | Sí | `add xD, xD, #imm` | Suma inmediata con destino explícito. |
| `sub` | `reg, reg` | Soportado | Sí | `sub xD, xD, xS` | Resta de registros (`xD = xD - xS`). |
| `sub` | `reg, imm` | Soportado | Sí | `sub xD, xD, #imm` | Resta con valor inmediato. |
| `and` | `reg, reg` | Soportado | Sí | `and xD, xD, xS` | Operación lógica AND a nivel de bits con tres operandos explícitos. |
| `and` | `reg, imm` | Soportado | Sí | `and xD, xD, #imm` | Operación lógica AND con constante inmediata. |
| `or` | `reg, reg` | Soportado | Sí | `orr xD, xD, xS` | Operación lógica OR inclusiva; mnemónico `orr` en AArch64. |
| `or` | `reg, imm` | Soportado | Sí | `orr xD, xD, #imm` | Operación lógica OR inclusiva con inmediato (`orr` en AArch64). |
| `xor` | `reg, reg` | Soportado | Sí | `eor xD, xD, xS` | Operación lógica XOR exclusiva; mnemónico `eor` en AArch64. |
| `xor` | `reg, imm` | Soportado | Sí | `eor xD, xD, #imm` | Operación lógica XOR exclusiva con inmediato (`eor` en AArch64). |
| `cmp` | `reg, reg` | Soportado | Sí | `cmp xD, xS` | Actualiza flags NZCV sin modificar registro destino (alias de `subs xzr, xD, xS`). |
| `cmp` | `reg, imm` | Soportado | Sí | `cmp xD, #imm` | Comparación de registro con inmediato de 64 bits. |
| `jmp` | `etiqueta` | Soportado | No | `b etiqueta` | Salto incondicional relativo (`b` en AArch64). |
| `je` | `etiqueta` | Soportado | No | `b.eq etiqueta` | Salto condicional si igual / cero (condición `ZF = 1`). |
| `jne` | `etiqueta` | Soportado | No | `b.ne etiqueta` | Salto condicional si no igual / no cero (condición `ZF = 0`). |
| `jl` | `etiqueta` | Soportado | No | `b.lt etiqueta` | Salto condicional si menor con signo (condición `SF != OF`). |
| `jle` | `etiqueta` | Soportado | No | `b.le etiqueta` | Salto condicional si menor o igual con signo (`ZF = 1` o `SF != OF`). |
| `jg` | `etiqueta` | Soportado | No | `b.gt etiqueta` | Salto condicional si mayor con signo (`ZF = 0` y `SF == OF`). |
| `jge` | `etiqueta` | Soportado | No | `b.ge etiqueta` | Salto condicional si mayor o igual con signo (`SF == OF`). |
| `call` | `etiqueta` | Soportado | No | `str x30, [sp, #-16]!; bl etiqueta; ldr x30, [sp], #16` | Preserva el enlace del llamante en un bloque temporal de 16 bytes. Sólo llamadas internas con stack restablecido al retornar. |
| `push` | `reg` | Soportado | No | `str xReg, [sp, #-16]!` | Registro distinto de `rsp`; almacenamiento con pre-indexado preservando alineación de 16 bytes. |
| `pop` | `reg` | Soportado | No | `ldr xReg, [sp], #16` | Registro distinto de `rsp`; carga con post-indexado restaurando el puntero de pila. |
| `ret` | *(sin operandos)* | Soportado | No | `ret` | Retorno de subrutina (en AArch64 salta a la dirección en `x30`/`lr`). |

---

## 3. Instrucciones fuera del subconjunto soportado

Las siguientes instrucciones no tienen reglas de traducción implementadas y se rechazan como no soportadas (`E001`):
- Operaciones aritméticas adicionales (`imul`, `idiv`, `inc`, `dec`, `neg`).
- Operaciones de desplazamiento y rotación (`shl`, `shr`, `sar`).
- Comparaciones y saltos sin signo (`ja`, `jb`, `jae`, `jbe`).
- Desplazamiento condicional (`cmovcc`).
