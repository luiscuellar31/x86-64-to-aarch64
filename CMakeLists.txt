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
