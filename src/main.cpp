#include <iostream>
#include "instruccion.hpp"
#include "normalizador.hpp"
#include "analizador_sintactico.hpp"

int main() {
    std::cout << "=== Demostracion del Traductor x86-64 a AArch64 ===\n\n";

    // 1. Demostracion del modelo de datos de instruccion
    std::cout << "[1] Modelo de datos interno (Fase 0):\n";
    Instruccion instruccionEjemplo(CodigoOperacion::Add, 1);
    instruccionEjemplo.agregarOperando(Operando::crearRegistro("rax"));
    instruccionEjemplo.agregarOperando(Operando::crearInmediato("5"));
    instruccionEjemplo.imprimir();

    // 2. Demostracion del normalizador de texto (Fase 2, Subfase 1)
    std::cout << "\n[2] Normalizacion de linea (Fase 2, Subfase 1):\n";
    std::string lineaEntrada = "    ADD RAX, 5       ; incremento";
    std::string lineaNormalizada = Normalizador::normalizarLinea(lineaEntrada);
    std::cout << "  Entrada cruda:     \"" << lineaEntrada << "\"\n";
    std::cout << "  Salida normalizada: \"" << lineaNormalizada << "\"\n";

    // 3. Demostracion del analizador sintactico lineal (Fase 2, Subfase 2)
    std::cout << "\n[3] Analisis sintactico (Fase 2, Subfase 2):\n";
    std::string programaPrueba =
        "inicio:\n"
        "    mov rax, 42\n"
        "    add rax, 5\n"
        "    ret\n";
    std::cout << "  Programa de prueba:\n" << programaPrueba << "\n";
    std::vector<Instruccion> instrucciones = AnalizadorSintactico::analizarPrograma(programaPrueba);
    std::cout << "  Instrucciones reconocidas:\n";
    for (const auto& inst : instrucciones) {
        inst.imprimir();
    }

    std::cout << "\nComprobacion finalizada con exito.\n";
    return 0;
}