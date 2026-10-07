# se define la version de cmake requerida
cmake_minimum_required(VERSION 3.22)

# definimos nombre del proyecto, lenguajes y su version
project(x86_64_to_aarch64 LANGUAGES CXX VERSION 0.0.1)

# fijamos la version a C++ 17
set(CMAKE_CXX_STANDARD 17)
# pedimos que sea requerida
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# se añade el ejecutable primero y
# despues los archivos .cpp
add_executable(x86_64_to_aarch64
    src/main.cpp
)
