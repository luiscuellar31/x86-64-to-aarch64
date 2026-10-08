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

# Definicion del ejecutable principal y sus fuentes
add_executable(x86_64_to_aarch64
    src/main.cpp
    src/instruccion.cpp
    src/normalizador.cpp
    src/tabla_registros.cpp
    src/analizador_sintactico.cpp
)

target_include_directories(x86_64_to_aarch64 PRIVATE
    include
)

# Soporte de pruebas automatizadas
enable_testing()

add_executable(test_normalizador
    tests/test_normalizador.cpp
    src/normalizador.cpp
    src/tabla_registros.cpp
)

target_include_directories(test_normalizador PRIVATE
    include
)

add_test(NAME TestNormalizador COMMAND test_normalizador)

add_executable(test_analizador_sintactico
    tests/test_analizador_sintactico.cpp
    src/analizador_sintactico.cpp
    src/normalizador.cpp
    src/tabla_registros.cpp
    src/instruccion.cpp
)

target_include_directories(test_analizador_sintactico PRIVATE
    include
)

add_test(NAME TestAnalizadorSintactico COMMAND test_analizador_sintactico)
