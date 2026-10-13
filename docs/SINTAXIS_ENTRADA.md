# Especificación de Sintaxis de Entrada

Este documento define la sintaxis formal y las reglas léxicas del subconjunto de ensamblador x86-64 aceptado por el traductor en sus etapas iniciales (V0/V1).

---

## 1. Dialecto y Formato General

- **Estilo:** Intel / NASM simplificado (formato `instrucción destino, fuente`).
- **Codificación:** UTF-8 / ASCII plano.
- **Distinción entre mayúsculas y minúsculas (*case sensitivity*):**
  - **Mnemónicos y registros:** No distinguen mayúsculas de minúsculas. Se normalizan internamente a minúsculas (`MOV`, `Mov` y `mov` son equivalentes).
  - **Etiquetas:** Distinguen mayúsculas de minúsculas (`inicio:` es diferente de `Inicio:`).
- **Separadores:** Espacios en blanco (` `) o tabulaciones (`\t`) separan el mnemónico de los operandos. Una coma (`,`) separa estrictamente los operandos entre sí.

---

## 2. Comentarios y Líneas Vacías

- Los comentarios inician con un punto y coma (`;`) y abarcan hasta el final de la línea.
- Las líneas vacías o compuestas únicamente por espacios/comentarios se descartan en la etapa de normalización, conservando el contador de número de línea original para diagnósticos.

```asm
; Esto es un comentario de línea completa
mov rax, 10    ; Comentario al final de instrucción
```

---

## 3. Identificadores y Etiquetas

- Las etiquetas deben iniciar con una letra (`a-z`, `A-Z`) o guion bajo (`_`), seguidas de caracteres alfanuméricos o guiones bajos: `^[a-zA-Z_][a-zA-Z0-9_]*$`.
- Las etiquetas van seguidas de dos puntos (`:`).
- Los nombres de registros se reservan para operandos y no se admiten como etiquetas.
- Pueden presentarse en su propia línea o precediendo a una instrucción:

```asm
inicio:
    mov rax, 10

bucle: add rax, 1
```

---

## 4. Literales Numéricos (Inmediatos)

- **Enteros decimales:** Con o sin signo explícito (`10`, `-5`, `0`, `+42`).
- **Enteros hexadecimales:** Prefijo `0x` o `0X` seguido de dígitos hexadecimales (`0x1F`, `0x20`).
- Tamaño admitido inicialmente: valores enteros con signo que caben en 64 bits (`int64_t`).
- El signo debe estar unido al literal. Los decimales con ceros iniciales siguen siendo decimales (`010` representa 10 en el AST); no se interpreta notación octal.
- Los literales incompletos o fuera de rango se rechazan; nunca se sustituyen por cero. La codificación de estos valores en instrucciones AArch64 todavía requiere consolidación.

---

## 5. Operandos de Memoria Local Controlada

- **Formato soportado:** Acceso a variables locales y marco de pila mediante direccionamiento base y desplazamiento constante: `[base]`, `[base - imm]`, `[base + imm]`.
- **Registros base admitidos:** Registros de puntero de 64 bits (`rbp`, `rsp`).
- **Ancho de palabra:** Accesos estándar de 64 bits hacia/desde registros enteros.
- Las bases generales como `rax` o `rdi` y los anchos distintos de 64 bits se rechazan con `E014`. El ancho también se comprueba para instrucciones construidas mediante la biblioteca.
- **Instrucción admitida:** Exclusivamente en `mov`:
  - Carga (*load*): `mov reg, [rbp - imm]` → `ldr reg, [x29, #-imm]`
  - Almacenamiento (*store*): `mov [rbp - imm], reg` → `str reg, [x29, #-imm]`
- **Direccionamiento complejo fuera de alcance:** Formas con escalado, índice o direccionamiento relativo al puntero de instrucción (ej. `[rax + rbx*4 + 8]`, `[rip + etiqueta]`, `[base + index*scale + disp]`) se rechazan en parseo con `E030`. Las formas completas que el subconjunto no admite, como `mov mem, mem` o `mov mem, imm`, se rechazan en validación con `E014`.
- Un desplazamiento exige un único operador `+` o `-`, unido al número o separado por espacios. Se rechazan `[rbp 8]`, `[rbp - -8]`, corchetes incompletos y desplazamientos fuera de `int64_t`.

---

## 6. Directivas de Ensamblador

- Las directivas de ensamblador tradicionales (como `global`, `section .text`, `default rel`, etc.) todavía no están implementadas. Se rechazan como instrucciones no soportadas (`E001`); no se ignoran durante la normalización.

---

## 7. Sintaxis Excluida o Rechazada

Queda explícitamente fuera de soporte:
- **Sintaxis AT&T:** (ej. `movq $10, %rax` o `addq %rbx, %rax`).
- **Instrucciones con más de dos operandos** en x86-64.
- **Prefijos de segmento o bloqueos de bus:** (`lock`, `fs:`, `gs:`).

La normalización conserva las comas vacías o sobrantes para que el parser las rechace. Los errores de sintaxis producen `E030`, y los inmediatos incompletos o desbordados producen `E024`. Se conserva la línea original, y un programa rechazado no genera salida AArch64 parcial.
