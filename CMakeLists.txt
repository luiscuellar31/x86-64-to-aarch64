# Version minima de CMake requerida
cmake_minimum_required(VERSION 3.22)

# Nombre del proyecto, lenguaje y version inicial
project(x86_64_to_aarch64 LANGUAGES CXX VERSION 0.0.1)

# Estandar de C++17 requerido
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Advertencias utiles del compilador para asegurar calidad de codigo
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# -----------------------------------------------------------------------------
# Biblioteca estática con el núcleo del traductor (translator_core)
# -----------------------------------------------------------------------------
add_library(traductor_nucleo STATIC
    src/instruccion.cpp
    src/normalizador.cpp
    src/tabla_registros.cpp
    src/analizador_lexico.cpp
    src/analizador_sintactico.cpp
    src/ir_aarch64.cpp
    src/diagnostico.cpp
    src/resultado_traduccion.cpp
    src/validador.cpp
    src/traductor.cpp
    src/opciones_cli.cpp
)

target_include_directories(traductor_nucleo PUBLIC
    include
)

# -----------------------------------------------------------------------------
# Interfaz de línea de comandos (CLI)
# -----------------------------------------------------------------------------
add_executable(x86_64_to_aarch64
    src/main.cpp
)

target_link_libraries(x86_64_to_aarch64 PRIVATE
    traductor_nucleo
)

# -----------------------------------------------------------------------------
# Pruebas automatizadas (todas consumen traductor_nucleo)
# -----------------------------------------------------------------------------
enable_testing()

add_executable(test_normalizador tests/test_normalizador.cpp)
target_link_libraries(test_normalizador PRIVATE traductor_nucleo)
add_test(NAME TestNormalizador COMMAND test_normalizador)

add_executable(test_analizador_lexico tests/test_analizador_lexico.cpp)
target_link_libraries(test_analizador_lexico PRIVATE traductor_nucleo)
add_test(NAME TestAnalizadorLexico COMMAND test_analizador_lexico)

add_executable(test_analizador_sintactico tests/test_analizador_sintactico.cpp)
target_link_libraries(test_analizador_sintactico PRIVATE traductor_nucleo)
add_test(NAME TestAnalizadorSintactico COMMAND test_analizador_sintactico)

add_executable(test_ir tests/test_ir.cpp)
target_link_libraries(test_ir PRIVATE traductor_nucleo)
add_test(NAME TestIR COMMAND test_ir)

add_executable(test_diagnostico tests/test_diagnostico.cpp)
target_link_libraries(test_diagnostico PRIVATE traductor_nucleo)
add_test(NAME TestDiagnostico COMMAND test_diagnostico)

add_executable(test_resultado_traduccion tests/test_resultado_traduccion.cpp)
target_link_libraries(test_resultado_traduccion PRIVATE traductor_nucleo)
add_test(NAME TestResultadoTraduccion COMMAND test_resultado_traduccion)

add_executable(test_validador tests/test_validador.cpp)
target_link_libraries(test_validador PRIVATE traductor_nucleo)
add_test(NAME TestValidador COMMAND test_validador)

add_executable(test_traductor tests/test_traductor.cpp)
target_link_libraries(test_traductor PRIVATE traductor_nucleo)
add_test(NAME TestTraductor COMMAND test_traductor)

add_executable(test_cli tests/test_cli.cpp)
target_link_libraries(test_cli PRIVATE traductor_nucleo)
target_compile_definitions(test_cli PRIVATE
    DIRECTORIO_EJEMPLOS_INVALIDOS="${CMAKE_CURRENT_SOURCE_DIR}/examples/invalidos"
)
add_test(NAME TestCLI COMMAND test_cli)

# Comprobar el corpus en AArch64 sin depender de la arquitectura del host.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND
   NOT CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(destinoEnsamblado "aarch64-unknown-linux-gnu")
    set(carpetaComprobacion "${CMAKE_CURRENT_BINARY_DIR}/comprobar_ensamblador_aarch64")
    file(MAKE_DIRECTORY "${carpetaComprobacion}")
    file(WRITE "${carpetaComprobacion}/entrada.s" "    mov x0, #1\n    ret\n")
    execute_process(
        COMMAND "${CMAKE_CXX_COMPILER}" -fintegrated-as -target "${destinoEnsamblado}"
            -c "${carpetaComprobacion}/entrada.s" -o "${carpetaComprobacion}/entrada.o"
        RESULT_VARIABLE resultadoComprobacion
        ERROR_VARIABLE errorComprobacion
        TIMEOUT 5
    )

    if(resultadoComprobacion STREQUAL "0")
        file(GLOB ejemplosValidos CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/examples/validos/*.s")
        if(NOT ejemplosValidos)
            message(FATAL_ERROR "No se encontraron ejemplos para comprobar el ensamblado AArch64.")
        endif()
        foreach(archivoEjemplo IN LISTS ejemplosValidos)
            get_filename_component(nombreEjemplo "${archivoEjemplo}" NAME_WE)
            set(nombrePrueba "TestEnsamblado_${nombreEjemplo}")
            add_test(NAME "${nombrePrueba}" COMMAND ${CMAKE_COMMAND}
                "-DTRADUCTOR=$<TARGET_FILE:x86_64_to_aarch64>"
                "-DENTRADA=${archivoEjemplo}"
                "-DCOMPILADOR=${CMAKE_CXX_COMPILER}"
                "-DDESTINO=${destinoEnsamblado}"
                "-DCARPETA_PRUEBA=${CMAKE_CURRENT_BINARY_DIR}/pruebas_ensamblado/${nombreEjemplo}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/verificar_ensamblado_aarch64.cmake"
            )
            set_tests_properties("${nombrePrueba}" PROPERTIES TIMEOUT 15)
        endforeach()
        message(STATUS "Pruebas de ensamblado AArch64 disponibles para el corpus válido.")
    else()
        message(STATUS "Ensamblado AArch64 no disponible en el compilador configurado: ${errorComprobacion}")
    endif()
else()
    message(STATUS "Las pruebas de ensamblado AArch64 requieren Clang con interfaz de línea de comandos GNU.")
endif()

# Ejecutar el ensamblador traducido en el host soportado, con un tiempo limite.
if(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$" AND NOT CMAKE_CROSSCOMPILING)
    add_test(NAME TestLlamadasNativas COMMAND ${CMAKE_COMMAND}
        "-DTRADUCTOR=$<TARGET_FILE:x86_64_to_aarch64>"
        "-DENTRADA=${CMAKE_CURRENT_SOURCE_DIR}/examples/validos/09_llamadas_anidadas.s"
        "-DCOMPILADOR=${CMAKE_CXX_COMPILER}"
        "-DCARPETA_PRUEBA=${CMAKE_CURRENT_BINARY_DIR}/prueba_llamadas_nativas"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/verificar_llamadas_nativas.cmake"
    )
    set_tests_properties(TestLlamadasNativas PROPERTIES TIMEOUT 20)

    set(carpetaReferencia "${CMAKE_CURRENT_BINARY_DIR}/comprobar_ejecucion_x86")
    file(MAKE_DIRECTORY "${carpetaReferencia}")
    file(WRITE "${carpetaReferencia}/comprobar.cpp" "int main() { return 0; }\n")
    execute_process(
        COMMAND "${CMAKE_CXX_COMPILER}" -std=c++17 -arch x86_64
            "${carpetaReferencia}/comprobar.cpp" -o "${carpetaReferencia}/comprobar"
        RESULT_VARIABLE resultadoCompilacionX86
        ERROR_VARIABLE errorCompilacionX86
        TIMEOUT 10
    )
    if(resultadoCompilacionX86 STREQUAL "0")
        execute_process(
            COMMAND "${carpetaReferencia}/comprobar"
            RESULT_VARIABLE resultadoEjecucionX86
            ERROR_VARIABLE errorEjecucionX86
            TIMEOUT 5
        )
        if(resultadoEjecucionX86 STREQUAL "0")
            file(GLOB ejemplosFuncionales CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/funcionales/*.s")
            if(NOT ejemplosFuncionales)
                message(FATAL_ERROR "No se encontraron funciones para comparar x86-64 y AArch64.")
            endif()
            foreach(archivoFuncion IN LISTS ejemplosFuncionales)
                get_filename_component(nombreFuncion "${archivoFuncion}" NAME_WE)
                set(nombrePrueba "TestFuncional_${nombreFuncion}")
                add_test(NAME "${nombrePrueba}" COMMAND ${CMAKE_COMMAND}
                    "-DTRADUCTOR=$<TARGET_FILE:x86_64_to_aarch64>"
                    "-DENTRADA=${archivoFuncion}"
                    "-DCOMPILADOR=${CMAKE_CXX_COMPILER}"
                    "-DPROGRAMA_PRUEBA=${CMAKE_CURRENT_SOURCE_DIR}/tests/comparar_funciones.cpp"
                    "-DCARPETA_PRUEBA=${CMAKE_CURRENT_BINARY_DIR}/pruebas_funcionales/${nombreFuncion}"
                    -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/verificar_funciones.cmake"
                )
                set_tests_properties("${nombrePrueba}" PROPERTIES TIMEOUT 40)
            endforeach()
            message(STATUS "Comparación funcional x86-64/AArch64 disponible en este host.")
        else()
            message(STATUS "Ejecución x86-64 no disponible: ${resultadoEjecucionX86} ${errorEjecucionX86}")
        endif()
    else()
        message(STATUS "Compilación x86-64 no disponible: ${resultadoCompilacionX86} ${errorCompilacionX86}")
    endif()
endif()

# Las comprobaciones con assert deben ejecutarse tambien en Release.
set(objetivosPrueba
    test_normalizador test_analizador_lexico test_analizador_sintactico
    test_ir test_diagnostico test_resultado_traduccion test_validador
    test_traductor test_cli
)

option(CONSTRUIR_GUI "Construir la interfaz gráfica con Qt Widgets" OFF)
if(CONSTRUIR_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    add_executable(x86_64_to_aarch64_gui src/gui/main_gui.cpp src/gui/ventana_traductor.cpp)
    target_include_directories(x86_64_to_aarch64_gui PRIVATE src/gui)
    target_link_libraries(x86_64_to_aarch64_gui PRIVATE traductor_nucleo Qt6::Widgets)

    add_executable(test_ventana_traductor tests/test_ventana_traductor.cpp src/gui/ventana_traductor.cpp)
    target_include_directories(test_ventana_traductor PRIVATE src/gui)
    target_link_libraries(test_ventana_traductor PRIVATE traductor_nucleo Qt6::Widgets)
    add_test(NAME TestVentanaTraductor COMMAND test_ventana_traductor
        "${CMAKE_CURRENT_BINARY_DIR}/ventana_traductor.png")
    set_tests_properties(TestVentanaTraductor PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 15)
    list(APPEND objetivosPrueba test_ventana_traductor)
endif()

foreach(objetivoPrueba IN LISTS objetivosPrueba)
    if(MSVC)
        target_compile_options(${objetivoPrueba} PRIVATE /UNDEBUG)
    else()
        target_compile_options(${objetivoPrueba} PRIVATE -UNDEBUG)
    endif()
endforeach()

add_executable(medir_corpus EXCLUDE_FROM_ALL tests/medir_corpus.cpp)
target_link_libraries(medir_corpus PRIVATE traductor_nucleo)

add_custom_target(metricas
    COMMAND ${CMAKE_COMMAND}
        "-DMEDIDOR=$<TARGET_FILE:medir_corpus>"
        "-DCTEST=${CMAKE_CTEST_COMMAND}"
        "-DRAIZ_PROYECTO=${CMAKE_CURRENT_SOURCE_DIR}"
        "-DCARPETA_BUILD=${CMAKE_CURRENT_BINARY_DIR}"
        "-DCONFIGURACION=$<CONFIG>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/registrar_metricas.cmake"
    DEPENDS medir_corpus x86_64_to_aarch64 ${objetivosPrueba}
    USES_TERMINAL
)

