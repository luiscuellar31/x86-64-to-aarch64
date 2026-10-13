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

execute_process(
    COMMAND "${COMPILADOR}" -fintegrated-as -target "${DESTINO}"
        -c "${CARPETA_PRUEBA}/traducido.s" -o "${CARPETA_PRUEBA}/traducido.o"
    RESULT_VARIABLE resultadoEnsamblado
    ERROR_VARIABLE errorEnsamblado
    TIMEOUT 5
)
if(NOT resultadoEnsamblado STREQUAL "0")
    message(FATAL_ERROR "Fallo al ensamblar ${ENTRADA} (${resultadoEnsamblado}):\n${errorEnsamblado}")
endif()
