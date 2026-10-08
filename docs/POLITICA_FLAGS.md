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

## 3. Política para Saltos y Comparaciones (Milestone 3.2)

### 3.1. Semántica y efecto de `cmp`
- **En x86-64:** `cmp op1, op2` ejecuta internamente la resta aritmética `op1 - op2`, descartando el resultado numérico y actualizando las banderas de estado del registro `RFLAGS`:
  - `ZF` (*Zero Flag*): se activa si `op1 == op2` (resultado cero).
  - `SF` (*Sign Flag*): refleja el bit más significativo (signo) del resultado.
  - `OF` (*Overflow Flag*): se activa si hubo desbordamiento aritmético con signo.
  - `CF` (*Carry Flag*): refleja acarreo o préstamo en aritmética sin signo.
- **En AArch64:** La instrucción `cmp xD, xS/#imm` es un alias arquitectónico estándar de `subs xzr, xD, xS/#imm`. Realiza la resta, descarta el resultado escribiendo al registro nulo `xzr` y actualiza las cuatro banderas de condición de `PSTATE`:
  - `N` (*Negative*): resultado negativo con signo.
  - `Z` (*Zero*): resultado igual a cero.
  - `C` (*Carry*): acarreo sin signo.
  - `V` (*oVerflow*): desbordamiento con signo.

### 3.2. Correspondencia de saltos condicionales con signo (*signed*)
Las condiciones con signo se mapean de forma unívoca y determinista entre ambas arquitecturas:

| Instrucción x86-64 | Condición x86-64 | Instrucción AArch64 | Condición `PSTATE` | Significado pedagógico |
| :--- | :--- | :--- | :--- | :--- |
| `je etiqueta` | `ZF == 1` | `b.eq etiqueta` | `Z == 1` | Salto si son iguales. |
| `jne etiqueta` | `ZF == 0` | `b.ne etiqueta` | `Z == 0` | Salto si son diferentes. |
| `jl etiqueta` | `SF != OF` | `b.lt etiqueta` | `N != V` | Salto si menor con signo. |
| `jle etiqueta` | `ZF == 1` o `SF != OF` | `b.le etiqueta` | `Z == 1` o `N != V` | Salto si menor o igual con signo. |
| `jg etiqueta` | `ZF == 0` y `SF == OF` | `b.gt etiqueta` | `Z == 0` y `N == V` | Salto si mayor con signo. |
| `jge etiqueta` | `SF == OF` | `b.ge etiqueta` | `N == V` | Salto si mayor o igual con signo. |
| `jmp etiqueta` | Incondicional | `b etiqueta` | Siempre | Bifurcación relativa incondicional. |

### 3.3. Justificación académica: comparaciones con signo (*signed*) vs. sin signo (*unsigned*)
- Las comparaciones sin signo en x86-64 (`ja`, `jb`, `jae`, `jbe`) dependen críticamente de la bandera `CF` (*Carry/Borrow*).
- Existe una diferencia arquitectónica sutil pero fundamental:
  - En x86-64, la resta activa `CF = 1` si hubo un préstamo (*borrow*).
  - En AArch64, la resta define el flag `C` como *not borrow* (`C = 1` si NO hubo préstamo, `C = 0` si hubo préstamo).
- Por este motivo, el subconjunto inicial soporta exclusivamente comparaciones enteras con signo (`signed`), donde la relación entre `N` (o `SF`) y `V` (o `OF`) es directamente isomórfica. Las instrucciones sin signo (`ja`, `jb`, etc.) se reservan para etapas posteriores cuando se incorpore la inversión del flag de acarreo en el modelo semántico.
