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
| `ret` | *(sin operandos)* | Soportado (V0) | No | `ret` | Retorno de subrutina (en AArch64 salta a la dirección en `x30`/`lr`). |

---

## 3. Matriz de Instrucciones Planificadas (Etapas Siguientes)

| Instrucción x86-64 | Forma | Estado | Afecta Flags x86 | Equivalente AArch64 | Notas |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `cmp` | `reg, reg / imm` | Planificado (M3.2) | Sí | `cmp xD, xS/#imm` | Actualiza flags NZCV sin modificar registro destino. |
| `jmp` | `etiqueta` | Planificado (M3.2) | No | `b etiqueta` | Salto incondicional relativo (`b` en AArch64). |
| `je` / `jne` | `etiqueta` | Planificado (M3.2) | No | `b.eq` / `b.ne etiqueta` | Saltos condicionales basados en condición cero/igualdad. |
| `jl` / `jle` | `etiqueta` | Planificado (M3.2) | No | `b.lt` / `b.le etiqueta` | Saltos condicionales con signo (menor / menor o igual). |
| `jg` / `jge` | `etiqueta` | Planificado (M3.2) | No | `b.gt` / `b.ge etiqueta` | Saltos condicionales con signo (mayor / mayor o igual). |
