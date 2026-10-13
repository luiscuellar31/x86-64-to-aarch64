set(carpetaMetricas "${CARPETA_BUILD}/metricas")
file(MAKE_DIRECTORY "${carpetaMetricas}")

execute_process(
    COMMAND "${MEDIDOR}" "${RAIZ_PROYECTO}"
    OUTPUT_FILE "${carpetaMetricas}/CORPUS.md"
    ERROR_VARIABLE errorMedicion
    RESULT_VARIABLE resultadoMedicion
    TIMEOUT 10
)

execute_process(
    COMMAND "${CTEST}" --test-dir "${CARPETA_BUILD}" --output-on-failure
        --build-config "${CONFIGURACION}"
        --output-junit "${carpetaMetricas}/pruebas.xml"
        --output-log "${carpetaMetricas}/pruebas.log"
    RESULT_VARIABLE resultadoPruebas
    TIMEOUT 300
)

if(NOT resultadoMedicion STREQUAL "0")
    message(FATAL_ERROR "Fallo al medir el corpus (${resultadoMedicion}): ${errorMedicion}")
endif()
if(NOT resultadoPruebas STREQUAL "0")
    message(FATAL_ERROR "Las pruebas no se aprobaron (${resultadoPruebas}); consulte ${carpetaMetricas}/pruebas.log.")
endif()

message(STATUS "Métricas del corpus y resultados de pruebas registrados en ${carpetaMetricas}.")
