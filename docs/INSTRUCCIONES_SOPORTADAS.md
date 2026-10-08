# Matriz de Instrucciones Soportadas

Este documento define la matriz técnica de correspondencia entre el subconjunto de x86-64 y AArch64.

---

## 1. Alcance de Ancho de Palabra (64 bits)

En las etapas V0 y V1, el traductor opera **únicamente con registros y operaciones enteras de 64 bits**.
- **Soportados:** `rax`, `rbx`, `rcx`, `rdx`, `rsi`, `rdi`, `rbp`, `rsp`, `r8` a `r15`.
- **Excluidos inicialmente:** Subregistros de 32, 16 y 8 bits (`eax`, `ax`, `al`, `ah`, etc.) para evitar complejidad prematura con extensión de ceros (*zero-extension*) y escrituras parciales.

---

## 2. Matriz de Instrucciones (Etapa V0 / MVP)

| Instrucción x86-64 | Forma | Estado | Afecta Flags x86 | Equivalente AArch64 | Notas y Explicación Didáctica |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `mov` | `reg, reg` | Soportado (V0) | No | `mov xD, xS` | Copia directa de registro a registro de 64 bits. |
| `mov` | `reg, imm` | Soportado (V0) | No | `mov xD, #imm` | Carga de constante inmediata en registro. |
| `add` | `reg, reg` | Soportado (V0) | Sí | `add xD, xD, xS` | x86 usa destino destructivo de 2 operandos; AArch64 expresa 3 operandos (`destino, fuente1, fuente2`). |
| `add` | `reg, imm` | Soportado (V0) | Sí | `add xD, xD, #imm` | Suma inmediata con destino explícito. |
| `sub` | `reg, reg` | Soportado (V0) | Sí | `sub xD, xD, xS` | Resta de registros (`xD = xD - xS`). |
| `sub` | `reg, imm` | Soportado (V0) | Sí | `sub xD, xD, #imm` | Resta con valor inmediato. |
| `and` | `reg, reg` | Soportado (M3.1) | Sí | `and xD, xD, xS` | Operación lógica AND a nivel de bits con tres operandos explícitos. |
| `and` | `reg, imm` | Soportado (M3.1) | Sí | `and xD, xD, #imm` | Operación lógica AND con constante inmediata. |
| `or` | `reg, reg` | Soportado (M3.1) | Sí | `orr xD, xD, xS` | Operación lógica OR inclusiva; mnemónico `orr` en AArch64. |
| `or` | `reg, imm` | Soportado (M3.1) | Sí | `orr xD, xD, #imm` | Operación lógica OR inclusiva con inmediato (`orr` en AArch64). |
| `xor` | `reg, reg` | Soportado (M3.1) | Sí | `eor xD, xD, xS` | Operación lógica XOR exclusiva; mnemónico `eor` en AArch64. |
| `xor` | `reg, imm` | Soportado (M3.1) | Sí | `eor xD, xD, #imm` | Operación lógica XOR exclusiva con inmediato (`eor` en AArch64). |
| `cmp` | `reg, reg` | Soportado (M3.2) | Sí | `cmp xD, xS` | Actualiza flags NZCV sin modificar registro destino (alias de `subs xzr, xD, xS`). |
| `cmp` | `reg, imm` | Soportado (M3.2) | Sí | `cmp xD, #imm` | Comparación de registro con inmediato de 64 bits. |
| `jmp` | `etiqueta` | Soportado (M3.2) | No | `b etiqueta` | Salto incondicional relativo (`b` en AArch64). |
| `je` | `etiqueta` | Soportado (M3.2) | No | `b.eq etiqueta` | Salto condicional si igual / cero (condición `ZF = 1`). |
| `jne` | `etiqueta` | Soportado (M3.2) | No | `b.ne etiqueta` | Salto condicional si no igual / no cero (condición `ZF = 0`). |
| `jl` | `etiqueta` | Soportado (M3.2) | No | `b.lt etiqueta` | Salto condicional si menor con signo (condición `SF != OF`). |
| `jle` | `etiqueta` | Soportado (M3.2) | No | `b.le etiqueta` | Salto condicional si menor o igual con signo (`ZF = 1` o `SF != OF`). |
| `jg` | `etiqueta` | Soportado (M3.2) | No | `b.gt etiqueta` | Salto condicional si mayor con signo (`ZF = 0` y `SF == OF`). |
| `jge` | `etiqueta` | Soportado (M3.2) | No | `b.ge etiqueta` | Salto condicional si mayor o igual con signo (`SF == OF`). |
| `ret` | *(sin operandos)* | Soportado (V0) | No | `ret` | Retorno de subrutina (en AArch64 salta a la dirección en `x30`/`lr`). |

---

## 3. Matriz de Instrucciones Planificadas (Etapas Siguientes)

| Instrucción x86-64 | Forma | Estado | Afecta Flags x86 | Equivalente AArch64 | Notas |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `mov` | `reg, [rbp - imm]` | Planificado (M3.3) | No | `ldr xD, [x29, #-imm]` | Carga desde marco de pila local. |
| `mov` | `[rbp - imm], reg` | Planificado (M3.3) | No | `str xS, [x29, #-imm]` | Almacenamiento en marco de pila local. |
| `call` | `etiqueta` | Planificado (M3.4) | Sí/No | `bl etiqueta` | Llamada a subrutina con enlace de retorno en `x30`. |
