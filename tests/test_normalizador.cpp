#include <iostream>
#include <cassert>
#include "normalizador.hpp"

void probarNormalizacionLineaIndividual() {
    std::cout << "[Test] Probando normalizacion de lineas individuales...\n";

    // 1. Linea vacia o solo espacios
    assert(Normalizador::normalizarLinea("    \t  ") == "");

    // 2. Comentario exclusivo
    assert(Normalizador::normalizarLinea("  ; comentario de prueba") == "");

    // 3. Instruccion con mayusculas, espacios y comentario
    assert(Normalizador::normalizarLinea("    ADD RAX, 5       ; incremento") == "add rax, 5");

    // 4. Copia con espaciado irregular alrededor de coma
    assert(Normalizador::normalizarLinea("  mov   rbx  ,   rax  ") == "mov rbx, rax");

    // 5. Instruccion ret simple con mayusculas
    assert(Normalizador::normalizarLinea("  RET  ; retornar") == "ret");

    // 6. Etiqueta sola preservando case
    assert(Normalizador::normalizarLinea("MiEtiqueta:") == "MiEtiqueta:");

    // 7. Etiqueta con instruccion en la misma linea
    assert(Normalizador::normalizarLinea("Inicio: MOV RAX, 10") == "Inicio: mov rax, 10");

    std::cout << "  -> Exito en normalizacion individual.\n";
}

void probarNormalizacionProgramaCompleto() {
    std::cout << "[Test] Probando normalizacion de programa completo con lineas de origen...\n";

    std::string codigoPrueba =
        "; Cabecera del programa\n"
        "\n"
        "inicio:\n"
        "    MOV RAX, 100    ; cargar valor\n"
        "    \n"
        "    ADD RAX, 25\n"
        "    SUB RAX, 5\n"
        "    RET\n";

    std::vector<LineaNormalizada> lineas = Normalizador::normalizarCodigo(codigoPrueba);

    // Debe contener 5 lineas utiles: inicio:, mov, add, sub, ret
    assert(lineas.size() == 5);

    // Verificamos numeros de linea originales
    assert(lineas[0].numeroLineaOriginal == 3);
    assert(lineas[0].contenido == "inicio:");

    assert(lineas[1].numeroLineaOriginal == 4);
    assert(lineas[1].contenido == "mov rax, 100");

    assert(lineas[2].numeroLineaOriginal == 6);
    assert(lineas[2].contenido == "add rax, 25");

    assert(lineas[3].numeroLineaOriginal == 7);
    assert(lineas[3].contenido == "sub rax, 5");

    assert(lineas[4].numeroLineaOriginal == 8);
    assert(lineas[4].contenido == "ret");

    std::cout << "  -> Exito en conservacion de lineas y filtrado.\n";
}

int main() {
    std::cout << "=== Ejecutando pruebas unitarias de Normalizador (Subfase 5.1) ===\n";
    probarNormalizacionLineaIndividual();
    probarNormalizacionProgramaCompleto();
    std::cout << "Todas las pruebas pasaron correctamente.\n";
    return 0;
}
