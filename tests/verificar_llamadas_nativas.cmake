# Prueba acotada para macOS arm64 con el compilador ya configurado en CMake.
file(MAKE_DIRECTORY "${CARPETA_PRUEBA}")

execute_process(
    COMMAND "${TRADUCTOR}" "${ENTRADA}" -o "${CARPETA_PRUEBA}/traducido.s"
    RESULT_VARIABLE resultadoTraduccion
    OUTPUT_VARIABLE salidaTraduccion
    ERROR_VARIABLE errorTraduccion
    TIMEOUT 5
)
if(NOT resultadoTraduccion STREQUAL "0")
    message(FATAL_ERROR "Fallo de traduccion: ${errorTraduccion}")
endif()

file(READ "${CARPETA_PRUEBA}/traducido.s" codigoTraducido)
# El wrapper sin argumentos permite ejecutar la convencion interna desde C++.
file(WRITE "${CARPETA_PRUEBA}/funciones.s"
    ".text\n.globl _comprobar_llamadas\n_comprobar_llamadas:\n    b inicio\n${codigoTraducido}"
)
file(WRITE "${CARPETA_PRUEBA}/comprobar.cpp" [=[
#include <iostream>
extern "C" long long comprobar_llamadas();

int main() {
    for (int repeticion = 0; repeticion < 3; repeticion++) {
        long long resultado = comprobar_llamadas();
        if (resultado != 15) {
            std::cerr << "Retorno incorrecto: " << resultado << " (esperado: 15)\n";
            return 1;
        }
    }
    return 0;
}
]=])

execute_process(
    COMMAND "${COMPILADOR}" -std=c++17 "${CARPETA_PRUEBA}/comprobar.cpp"
        "${CARPETA_PRUEBA}/funciones.s" -o "${CARPETA_PRUEBA}/comprobar"
    RESULT_VARIABLE resultadoCompilacion
    ERROR_VARIABLE errorCompilacion
    TIMEOUT 5
)
if(NOT resultadoCompilacion STREQUAL "0")
    message(FATAL_ERROR "Fallo de compilacion del harness: ${errorCompilacion}")
endif()

execute_process(
    COMMAND "${CARPETA_PRUEBA}/comprobar"
    RESULT_VARIABLE resultadoEjecucion
    ERROR_VARIABLE errorEjecucion
    TIMEOUT 5
)
if(NOT resultadoEjecucion STREQUAL "0")
    message(FATAL_ERROR "Fallo de retornos nativos (${resultadoEjecucion}): ${errorEjecucion}")
endif()
