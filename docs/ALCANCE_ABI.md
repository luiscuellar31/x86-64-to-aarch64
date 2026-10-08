# Alcance de Convención de Llamada (ABI)

Este documento delimita los supuestos de la interfaz binaria de aplicación (ABI) considerados para la traducción entre **x86-64 System V** y **AArch64 AAPCS64**.

---

## 1. Supuestos en la Etapa Inicial (V0)

- **Funciones hoja simples (Leaf Functions):** El soporte inicial se centra en secuencias lineales de código que terminan en `ret` sin invocar subrutinas adicionales (`call` no forma parte de V0).
- **Convención de retorno:** El valor de resultado de la función reside en `rax` en x86-64, el cual se corresponde directamente con `x0` en AArch64.
- **Pila / Stack:** No se requiere creación de marco de pila (*stack frame*) para rutinas que no guardan variables locales en memoria.

---

## 2. Reglas Proyectadas para Futuras Etapas (V1+)

Cuando se introduzca soporte para llamadas de subrutinas (`call`):

1. **Alineación de la pila:** En AArch64, el puntero de pila (`sp`) debe mantenerse estrictamente alineado a **16 bytes** en cualquier llamada externa o instrucción de acceso al stack.
2. **Link Register (`x30` / `lr`):** A diferencia de x86-64 (donde la instrucción `call` empuja automáticamente la dirección de retorno a la pila en memoria), AArch64 guarda la dirección de retorno en el registro `x30`. En funciones no-hoja, el prólogo deberá guardar `x29` y `x30` en el stack mediante `stp x29, x30, [sp, #-16]!`.
3. **Clasificación de Registros:**
   - **Volátiles (Caller-Saved):** `x0` a `x15`. Pueden ser modificados libremente por la función invocada.
   - **No volátiles (Callee-Saved):** `x19` a `x28`. Toda función que los modifique debe preservarlos y restaurarlos antes de retornar.
