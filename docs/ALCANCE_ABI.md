# Alcance de Convención de Llamada (ABI)

Este documento delimita los supuestos de la interfaz binaria de aplicación (ABI) considerados para la traducción entre **x86-64 System V** y **AArch64 AAPCS64**.

---

## 1. Supuestos en la Etapa Inicial (V0)

- **Funciones hoja simples (Leaf Functions):** El soporte inicial se centra en secuencias lineales de código que terminan en `ret` sin invocar subrutinas adicionales (`call` no forma parte de V0).
- **Convención de retorno:** El valor de resultado de la función reside en `rax` en x86-64, el cual se corresponde directamente con `x0` en AArch64.
- **Pila / Stack:** Se soportan funciones hoja con o sin marco de pila explícito (*stack frame*).

---

## 2. Gestión de Stack, Prólogo y Epílogo (Milestone 3.4)

### Alineación estricta de pila en AArch64 (AAPCS64)
- En **x86-64**, la instrucción `push` decrementa `rsp` en 8 bytes y `pop` lo incrementa en 8 bytes.
- En **AArch64**, la arquitectura de hardware impone una verificación estricta de alineación (*SP alignment check*): siempre que el puntero de pila (`sp`) interviene en una instrucción de memoria, su dirección debe ser múltiplo de **16 bytes**. Modificar `sp` en pasos impares de 8 bytes provocaría excepciones por fallo de alineación.

### Reglas de traducción para push y pop
Para preservar deterministamente la alineación a 16 bytes de la ABI sin depender de heurísticas complejas de análisis interprocedimental:
1. `push reg` se traduce a:
   ```asm
   str xReg, [sp, #-16]!
   ```
   (Almacenamiento con pre-indexado y actualización de `sp`, reservando un bloque alineado a 16 bytes).
2. `pop reg` se traduce a:
   ```asm
   ldr xReg, [sp], #16
   ```
   (Carga con post-indexado y actualización de `sp`, liberando el bloque de 16 bytes y restaurando el puntero alineado).

### Prólogo y epílogo estándar con Frame Pointer
La secuencia canónica de establecimiento y liberación de marco de pila:
```asm
push rbp
mov rbp, rsp
...
mov rsp, rbp
pop rbp
ret
```
Se traduce fielmente en AArch64 a:
```asm
str x29, [sp, #-16]!
mov x29, sp
...
mov sp, x29
ldr x29, [sp], #16
ret
```
Donde `rbp` mapea al registro de marco (*frame pointer*) `x29` y `rsp` al puntero de pila `sp`.

---

## 3. Soporte de Llamadas de Subrutinas (Milestone 3.5)

El traductor soporta llamadas directas a subrutina mediante `call etiqueta`, traduciéndolas a `bl etiqueta` en AArch64:

1. **Mecanismo de retorno y Link Register (`x30` / `lr`):**
   - En **x86-64**, la instrucción `call` decrementa `rsp` en 8 bytes y empuja automáticamente la dirección de retorno a la pila en memoria (`[rsp - 8]`).
   - En **AArch64**, la instrucción `bl` (*Branch with Link*) escribe la dirección de la siguiente instrucción directamente en el registro de enlace `x30` (`lr`) sin alterar la pila (`sp`).
2. **Diferenciación entre funciones hoja y no hoja:**
   - **Funciones hoja (*leaf functions*):** No invocan otras subrutinas. La dirección de retorno permanece intacta en `x30` durante toda la ejecución de la función y `ret` ejecuta el salto de retorno (`br x30`) sin necesidad de tocar la pila para la dirección de retorno.
   - **Funciones no hoja (*non-leaf functions*):** Si una subrutina invoca a otra mediante `call`/`bl`, la llamada anidada sobreescribirá el valor de `x30`. Por ende, la subrutina que actúa como llamante intermedia debe preservar `x30` (habitualmente junto con `x29` / frame pointer) en el marco de pila al inicio y restaurarlo antes de retornar.
3. **Paso de parámetros y convención de retorno:**
   - En **System V AMD64**, los primeros seis argumentos enteros se pasan en `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9`, y el valor de retorno en `rax`.
   - En **AAPCS64**, los argumentos se reciben en `x0` a `x7`, y el retorno en `x0`.
   - El mapeo biyectivo del traductor (`rax` → `x0`, `rdi` → `x1`, `rsi` → `x2`, etc.) asegura una correspondencia directa y determinista en llamadas simples.
4. **Clasificación de Registros:**
   - **Volátiles (*caller-saved* / preservados por el llamante):** `x0` a `x15`. Pueden ser modificados libremente por la función invocada.
   - **No volátiles (*callee-saved* / preservados por la función invocada):** `x19` a `x28`. Toda función que los modifique debe preservarlos y restaurarlos antes de retornar.
