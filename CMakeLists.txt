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
endif()

# Las comprobaciones con assert deben ejecutarse tambien en Release.
foreach(objetivoPrueba IN ITEMS
    test_normalizador test_analizador_lexico test_analizador_sintactico
    test_ir test_diagnostico test_resultado_traduccion test_validador
    test_traductor test_cli
)
    if(MSVC)
        target_compile_options(${objetivoPrueba} PRIVATE /UNDEBUG)
    else()
        target_compile_options(${objetivoPrueba} PRIVATE -UNDEBUG)
    endif()
endforeach()

