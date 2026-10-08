# Política de Registros

Este documento define la correspondencia determinista e inyectiva entre los registros generales de **x86-64** y **AArch64** para las primeras versiones del traductor.

---

## 1. Principio de Inyectividad

Para garantizar que un programa traducido mantenga la semántica sin colisiones accidentales de estado:
- Cada registro x86-64 debe mapearse a un registro AArch64 **único y distinto**.
- No se reutiliza el mismo registro destino de AArch64 para dos registros fuente de x86-64 en el subconjunto activo.

---

## 2. Tabla de Correspondencia (64 bits)

| Registro x86-64 | Registro AArch64 | Rol / Justificación |
| :--- | :--- | :--- |
| `rax` | `x0` | Acumulador principal / Valor de retorno primario |
| `rdi` | `x1` | 1.º argumento en convención System V / Registro general |
| `rsi` | `x2` | 2.º argumento en convención System V / Registro general |
| `rdx` | `x3` | 3.er argumento / Registro de datos general |
| `rcx` | `x4` | 4.º argumento / Contador o registro general |
| `r8`  | `x5` | 5.º argumento / Registro extendido |
| `r9`  | `x6` | 6.º argumento / Registro extendido |
| `r10` | `x7` | Registro temporal de llamada |
| `r11` | `x8` | Registro temporal de llamada |
| `rbx` | `x19` | Registro preservado (callee-saved) |
| `r12` | `x20` | Registro preservado (callee-saved) |
| `r13` | `x21` | Registro preservado (callee-saved) |
| `r14` | `x22` | Registro preservado (callee-saved) |
| `r15` | `x23` | Registro preservado (callee-saved) |
| `rbp` | `x29` | Frame Pointer (FP en AArch64) |
| `rsp` | `sp`  | Stack Pointer (SP en AArch64) |

---

## 3. Registros Reservados en AArch64

Ciertos registros en AArch64 quedan reservados para uso exclusivo de la arquitectura y del traductor:

- **`x30` (`lr` - Link Register):** Almacena la dirección de retorno en llamadas a funciones. La instrucción `ret` salta a la dirección contenida en este registro.
- **`x16` y `x17` (`ip0` / `ip1`):** Reservados como registros temporales del traductor para operaciones complejas futuras (como carga de inmediatos superiores a 16 bits mediante secuencias `movz`/`movk`).
- **`xzr` (Zero Register):** Registro especial de solo lectura que devuelve el valor `0` y descarta resultados de escrituras (utilizado internamente por instrucciones como `cmp`).
