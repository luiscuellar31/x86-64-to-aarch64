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

## 3. Reglas Proyectadas para Llamadas de Subrutinas (Milestone 3.5 / V1+)

Cuando se introduzca soporte para llamadas de subrutinas (`call`):

1. **Link Register (`x30` / `lr`):** A diferencia de x86-64 (donde la instrucción `call` empuja automáticamente la dirección de retorno a la pila en memoria), AArch64 guarda la dirección de retorno en el registro `x30`. En funciones no hoja (que realizan llamadas a otras subrutinas), el prólogo deberá guardar `x29` y `x30` en el stack mediante `stp x29, x30, [sp, #-16]!`.
2. **Clasificación de Registros:**
   - **Volátiles (*caller-saved* / preservados por el llamante):** `x0` a `x15`. Pueden ser modificados libremente por la función invocada.
   - **No volátiles (*callee-saved* / preservados por la función invocada):** `x19` a `x28`. Toda función que los modifique debe preservarlos y restaurarlos antes de retornar.
