#include <iostream>
#include "instruccion.hpp"
#include "normalizador.hpp"

int main() {
    std::cout << "=== Demostracion del Traductor x86-64 a AArch64 ===\n\n";

    // 1. Demostracion del modelo de datos de instruccion
    std::cout << "[1] Modelo de datos interno:\n";
    Instruccion instruccionEjemplo(CodigoOperacion::Add, 1);
    instruccionEjemplo.agregarOperando(Operando::crearRegistro("rax"));
    instruccionEjemplo.agregarOperando(Operando::crearInmediato("5"));
    instruccionEjemplo.imprimir();

    // 2. Demostracion del normalizador de texto (Subfase 5.1)
    std::cout << "\n[2] Normalizacion de linea (Subfase 5.1):\n";
    std::string lineaEntrada = "    ADD RAX, 5       ; incremento";
    std::string lineaNormalizada = Normalizador::normalizarLinea(lineaEntrada);
    std::cout << "  Entrada cruda:     \"" << lineaEntrada << "\"\n";
    std::cout << "  Salida normalizada: \"" << lineaNormalizada << "\"\n";

    std::cout << "\nComprobacion finalizada con exito.\n";
    return 0;
}