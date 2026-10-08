#include <iostream>
#include "instruccion.hpp"

int main() {
    std::cout << "=== Demostracion de Modelo de Datos (Fase 0) ===\n\n";

    // Creamos manualmente la instruccion: add rax, 5 (linea 1)
    Instruccion instruccionEjemplo(CodigoOperacion::Add, 1);
    instruccionEjemplo.agregarOperando(Operando::crearRegistro("rax"));
    instruccionEjemplo.agregarOperando(Operando::crearInmediato("5"));

    // Imprimimos la representacion interna para comprobar los datos
    instruccionEjemplo.imprimir();

    std::cout << "\nComprobacion finalizada con exito.\n";
    return 0;
}