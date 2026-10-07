# x86-64-to-aarch64

Prototipo de traductor estático de un subconjunto de ensamblador x86-64 textual a ensamblador AArch64 textual.

Este proyecto forma parte de una tesina orientada al diseño e implementación de una herramienta limitada, didáctica y evaluable para estudiar la traducción entre arquitecturas.

## Objetivo

Diseñar e implementar un traductor fuente-a-fuente que reciba código ensamblador x86-64 simplificado y genere código ensamblador AArch64 equivalente dentro de un subconjunto claramente definido.

## Alcance inicial

La primera versión del proyecto se enfocará en programas pequeños y autocontenidos, principalmente con operaciones enteras.

Subconjunto previsto:

- movimiento de datos;
- operaciones aritméticas básicas;
- comparaciones;
- saltos condicionales e incondicionales;
- etiquetas;
- manejo básico de stack;
- llamadas y retornos simples.

## Fuera de alcance

Por ahora, quedan fuera del proyecto:

- traducción binaria de ejecutables;
- soporte completo de la ISA x86-64;
- instrucciones SIMD, SSE, AVX o x87;
- syscalls complejas;
- enlazado dinámico;
- optimizaciones avanzadas;
- uso del LLM como mecanismo principal de traducción.

## Arquitectura prevista

```text
Código x86-64 textual
        ↓
Normalización
        ↓
Análisis léxico
        ↓
Análisis sintáctico
        ↓
Representación intermedia
        ↓
Validación
        ↓
Traducción
        ↓
Generación de código AArch64
```

## Estado actual

Etapa inicial del proyecto.

Pendiente:

- definir estructuras internas;
- implementar el parser inicial;
- implementar traducción básica para `mov`, `add`, `sub` y `ret`;
- crear pruebas con programas pequeños;
- documentar reglas de traducción.

## Licencia

Este proyecto se distribuye bajo la licencia Apache License 2.0. Consulta el archivo `LICENSE` para más información.