# Alcance de Convención de Llamada (ABI)

Este documento delimita los supuestos de la interfaz binaria de aplicación (ABI) considerados para la traducción entre **x86-64 System V** y **AArch64 AAPCS64**.

**Estado de implementación:** las llamadas internas guardan y restauran `x30` alrededor de cada `bl`. El traductor mantiene su convención interna de registros de 64 bits. La equivalencia de todos los desplazamientos de pila, el balance del stack y la interoperabilidad con funciones externas no quedan demostrados por esta corrección.

---

## 1. Convención interna de registros y retorno

- **Funciones hoja y llamadas internas:** Se admiten funciones que terminan en `ret`, tanto sin llamadas como con `call` a etiquetas del mismo programa. Las llamadas internas preservan el enlace de retorno según las reglas de la sección 3.
- **Convención de retorno:** El valor de resultado de la función reside en `rax` en x86-64, el cual se corresponde directamente con `x0` en AArch64.
- **Pila / Stack:** Se soportan funciones hoja con o sin marco de pila explícito (*stack frame*).

---

## 2. Gestión de pila, prólogo y epílogo

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

En estas dos reglas, `reg` debe ser un registro general de 64 bits distinto de `rsp`. El stack pointer puede actuar como base de memoria, pero la emisión actual no puede almacenarlo o cargarlo directamente como valor mediante `str`/`ldr`; `push rsp` y `pop rsp` se rechazan con `E014`.

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

## 3. Llamadas internas con preservación del enlace de retorno

El traductor soporta llamadas directas a etiquetas definidas en el mismo programa. Cada `call etiqueta` genera esta secuencia:

```asm
str x30, [sp, #-16]!
bl etiqueta
ldr x30, [sp], #16
```

Se reserva temporalmente un bloque de 16 bytes para el enlace del llamante, se ejecuta la llamada y se recupera el enlace al volver. La secuencia conserva la alineación previa de `sp`; exige una pila inicialmente alineada y que la subrutina restablezca su stack antes de retornar. Se aplica también a llamadas sucesivas y anidadas, sin analizar límites de funciones ni reescribir cada `ret`.

Esta regla cubre funciones internas que trabajan con los registros del subconjunto. El bloque temporal cambia el diseño de la pila frente a la dirección de retorno de 8 bytes de x86-64. Quedan fuera del soporte de llamadas internas los argumentos en pila, la lectura o modificación de la dirección de retorno y los accesos que dependan de ese diseño. El validador todavía no comprueba todos esos requisitos; deben respetarse en los programas del corpus.

Una etiqueta que precede a `call` se emite en el guardado de `x30`, para que un salto a esa etiqueta ejecute la secuencia completa. Las tres instrucciones comparten línea de origen, regla `CALL_DIRECT` y explicación; el mapeo registra las tres líneas destino.

1. **Mecanismo de retorno y Link Register (`x30` / `lr`):**
   - En **x86-64**, la instrucción `call` decrementa `rsp` en 8 bytes y empuja automáticamente la dirección de retorno a la pila en memoria (`[rsp - 8]`).
   - En **AArch64**, la instrucción `bl` (*Branch with Link*) escribe la dirección de la siguiente instrucción directamente en el registro de enlace `x30` (`lr`) sin alterar la pila (`sp`).
2. **Diferenciación entre funciones hoja y no hoja:**
   - **Funciones hoja (*leaf functions*):** No invocan otras subrutinas. La dirección de retorno permanece intacta en `x30` durante toda la ejecución de la función y `ret` ejecuta el salto de retorno (`br x30`) sin necesidad de tocar la pila para la dirección de retorno.
   - **Funciones no hoja (*non-leaf functions*):** Si una subrutina invoca a otra mediante `call`/`bl`, la llamada anidada sobreescribirá el valor de `x30`. En este prototipo se preserva antes de cada llamada y se restaura al volver, manteniendo el enlace original disponible para el `ret` de la función.
3. **Paso de parámetros y convención de retorno:**
   - En **System V AMD64**, los primeros seis argumentos enteros se pasan en `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9`, y el valor de retorno en `rax`.
   - En **AAPCS64**, los argumentos se reciben en `x0` a `x7`, y el retorno en `x0`.
   - El mapeo inyectivo del traductor (`rax` → `x0`, `rdi` → `x1`, `rsi` → `x2`, etc.) es una convención interna para código traducido. No adapta los argumentos a una interfaz pública AAPCS64: el primer argumento permanece en `x1`, mientras AAPCS64 utiliza `x0`. La interoperabilidad con funciones externas queda fuera del soporte de llamadas internas.
4. **Clasificación de Registros:**
   - **Volátiles (*caller-saved* / preservados por el llamante):** `x0` a `x15`. Pueden ser modificados libremente por la función invocada.
   - **No volátiles (*callee-saved* / preservados por la función invocada):** `x19` a `x28`. Toda función que los modifique debe preservarlos y restaurarlos antes de retornar.

### Verificación acotada

Las pruebas comparan la emisión exacta de llamadas simples, sucesivas y anidadas, con y sin frame pointer, y comprueban los mapeos de una línea a tres instrucciones. En macOS arm64, CTest ejecuta además `09_llamadas_anidadas.s` desde un wrapper sin argumentos: verifica el resultado 15 en tres invocaciones y aplica un tiempo límite para detectar retornos que no terminan. En otros hosts se conservan las pruebas de traducción, pero esta ejecución nativa no se registra. No constituye una prueba general de equivalencia con x86-64 ni de ABI externa.
