# Política de Tratamiento de Flags

Este documento define la estrategia para manejar las diferencias entre el registro de banderas de **x86-64 (RFLAGS)** y las banderas de condición de **AArch64 (PSTATE: N, Z, C, V)**.

---

## 1. Diferencia Arquitectónica Fundamental

- **En x86-64:** Prácticamente todas las instrucciones aritméticas y lógicas (`add`, `sub`, `and`, `or`, `xor`) modifican implícitamente las banderas de estado (`ZF`, `SF`, `CF`, `OF`, `PF`).
- **En AArch64:** Las instrucciones equivalentes básicas (`add`, `sub`, `and`, `orr`, `eor`) **no modifican las banderas por defecto**. Para alterarlas se requiere explícitamente el sufijo `s` (por ejemplo, `adds`, `subs`, `ands`).

---

## 2. Política para la Etapa Inicial (V0 y Milestone 3.1)

- Dado que en esta fase no se soportan saltos condicionales dependientes de flags aritméticos/lógicos, no existe dependencia posterior del estado de las banderas.
- **Aritmética básica (`add`, `sub`):**
  - Se traducen a sus versiones directas en AArch64:
    - `add rax, 5`  ⟶  `add x0, x0, #5`
    - `sub rax, rbx` ⟶  `sub x0, x0, x19`
- **Operaciones lógicas (`and`, `or`, `xor`):**
  - En x86-64, estas operaciones limpian incondicionalmente `CF` y `OF` (los ponen en 0), y actualizan `ZF`, `SF` y `PF`.
  - En AArch64, `and`, `orr` y `eor` preservan intacto el registro `NZCV`. Para actualizar banderas se requeriría `ands` (que actualiza `N` y `Z`, limpiando `C` y `V`), pero al no existir saltos dependientes, la emisión limpia sin sufijo `s` es la más eficiente y segura.
  - Traducciones directas emitidas:
    - `and rax, 15`   ⟶ `and x0, x0, #15`
    - `or  rax, rbx`  ⟶ `orr x0, x0, x19`
    - `xor rax, rax`  ⟶ `eor x0, x0, x0`
- Esta decisión evita ruido de instrucciones y mutaciones innecesarias del estado de la CPU en el código generado.

---

## 3. Política para Saltos y Comparaciones (V1)

Cuando se implemente control de flujo condicional:
1. La instrucción de comparación `cmp reg, op` en x86-64 se mapea a la instrucción `cmp xD, xS/#imm` en AArch64.
2. En AArch64, `cmp` es un alias arquitectónico de `subs xzr, xD, op` que actualiza los flags `NZCV` descartando el resultado numérico.
3. Las instrucciones de salto condicional se traducen inmediatamente a su equivalente directo:
   - `je` (Zero Flag activo) ⟶ `b.eq`
   - `jne` (Zero Flag inactivo) ⟶ `b.ne`
   - `jl` (Less than, con signo) ⟶ `b.lt`
   - `jle` (Less or equal, con signo) ⟶ `b.le`
   - `jg` (Greater than, con signo) ⟶ `b.gt`
   - `jge` (Greater or equal, con signo) ⟶ `b.ge`
