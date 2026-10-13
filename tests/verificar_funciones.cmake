file(MAKE_DIRECTORY "${CARPETA_PRUEBA}")

execute_process(
    COMMAND "${TRADUCTOR}" "${ENTRADA}" -o "${CARPETA_PRUEBA}/traducido.s"
    RESULT_VARIABLE resultadoTraduccion
    OUTPUT_VARIABLE salidaTraduccion
    ERROR_VARIABLE errorTraduccion
    TIMEOUT 5
)
if(NOT resultadoTraduccion STREQUAL "0")
    message(FATAL_ERROR "Fallo al traducir ${ENTRADA} (${resultadoTraduccion}):\n${errorTraduccion}")
endif()

file(READ "${ENTRADA}" codigoX86)
# El ensamblador de referencia interpreta punto y coma como separador de instrucciones.
string(REGEX REPLACE ";[^\n]*" "" codigoX86 "${codigoX86}")
file(WRITE "${CARPETA_PRUEBA}/referencia.s"
    ".intel_syntax noprefix\n.text\n.globl _evaluar\n_evaluar:\n${codigoX86}"
)

file(READ "${CARPETA_PRUEBA}/traducido.s" codigoAArch64)
# El arnes adapta dos argumentos de x0/x1 a la convencion interna x1/x2.
file(WRITE "${CARPETA_PRUEBA}/funcion_aarch64.s"
    ".text\n.globl _evaluar\n_evaluar:\n    mov x2, x1\n    mov x1, x0\n${codigoAArch64}"
)

execute_process(
    COMMAND "${COMPILADOR}" -std=c++17 -Wall -Wextra -Wpedantic -arch x86_64
        "${PROGRAMA_PRUEBA}" "${CARPETA_PRUEBA}/referencia.s" -o "${CARPETA_PRUEBA}/referencia"
    RESULT_VARIABLE resultadoCompilacionX86
    ERROR_VARIABLE errorCompilacionX86
    TIMEOUT 10
)
if(NOT resultadoCompilacionX86 STREQUAL "0")
    message(FATAL_ERROR "Fallo al compilar la referencia ${ENTRADA} (${resultadoCompilacionX86}):\n${errorCompilacionX86}")
endif()

execute_process(
    COMMAND "${COMPILADOR}" -std=c++17 -Wall -Wextra -Wpedantic -arch arm64
        "${PROGRAMA_PRUEBA}" "${CARPETA_PRUEBA}/funcion_aarch64.s" -o "${CARPETA_PRUEBA}/traducida"
    RESULT_VARIABLE resultadoCompilacionAArch64
    ERROR_VARIABLE errorCompilacionAArch64
    TIMEOUT 10
)
if(NOT resultadoCompilacionAArch64 STREQUAL "0")
    message(FATAL_ERROR "Fallo al compilar la traduccion ${ENTRADA} (${resultadoCompilacionAArch64}):\n${errorCompilacionAArch64}")
endif()

execute_process(
    COMMAND "${CARPETA_PRUEBA}/referencia"
    RESULT_VARIABLE resultadoEjecucionX86
    OUTPUT_VARIABLE salidaX86
    ERROR_VARIABLE errorEjecucionX86
    TIMEOUT 5
)
if(NOT resultadoEjecucionX86 STREQUAL "0")
    message(FATAL_ERROR "Fallo al ejecutar la referencia ${ENTRADA} (${resultadoEjecucionX86}):\n${errorEjecucionX86}")
endif()

execute_process(
    COMMAND "${CARPETA_PRUEBA}/traducida"
    RESULT_VARIABLE resultadoEjecucionAArch64
    OUTPUT_VARIABLE salidaAArch64
    ERROR_VARIABLE errorEjecucionAArch64
    TIMEOUT 5
)
if(NOT resultadoEjecucionAArch64 STREQUAL "0")
    message(FATAL_ERROR "Fallo al ejecutar la traduccion ${ENTRADA} (${resultadoEjecucionAArch64}):\n${errorEjecucionAArch64}")
endif()

if(salidaX86 STREQUAL "" OR salidaAArch64 STREQUAL "")
    message(FATAL_ERROR "La prueba no produjo resultados para ${ENTRADA}.")
endif()

if(NOT salidaX86 STREQUAL salidaAArch64)
    message(FATAL_ERROR "Resultados diferentes para ${ENTRADA}:\nx86-64:\n${salidaX86}\nAArch64:\n${salidaAArch64}")
endif()
