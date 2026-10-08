#include <iostream>
#include <cassert>
#include "validador.hpp"
#include "analizador_sintactico.hpp"

void probarCasosValidos() {
    std::cout << "[Test Gate] Probando instrucciones validas...\n";

    GestorDiagnosticos gestor;
    std::vector<Instruccion> validas = {
        AnalizadorSintactico::analizarTexto("mov rax, rbx", 1),
        AnalizadorSintactico::analizarTexto("mov rax, 42", 2),
        AnalizadorSintactico::analizarTexto("add rax, rbx", 3),
        AnalizadorSintactico::analizarTexto("add rax, 5", 4),
        AnalizadorSintactico::analizarTexto("sub rax, 1", 5),
        AnalizadorSintactico::analizarTexto("ret", 6),
        AnalizadorSintactico::analizarTexto("inicio:", 7),
        AnalizadorSintactico::analizarTexto("and rax, rbx", 8),
        AnalizadorSintactico::analizarTexto("and rax, 15", 9),
        AnalizadorSintactico::analizarTexto("or rbx, rcx", 10),
        AnalizadorSintactico::analizarTexto("or rbx, 1", 11),
        AnalizadorSintactico::analizarTexto("xor rax, rax", 12),
        AnalizadorSintactico::analizarTexto("xor rax, 255", 13)
    };

    bool resultado = Validador::validarPrograma(validas, gestor);
    assert(resultado == true);
    assert(!gestor.tieneErrores());
    assert(gestor.cantidadErrores() == 0);

    std::cout << "  -> Exito en casos validos.\n";
}

void probarRegistrosDesconocidos() {
    std::cout << "[Test Gate] Probando rechazo de registros no soportados o de 32 bits (E021)...\n";

    GestorDiagnosticos gestor;
    // 'eax' no es de 64 bits en nuestro subconjunto
    Instruccion instReg32 = AnalizadorSintactico::analizarTexto("mov eax, 10", 1);
    bool res = Validador::validarInstruccion(instReg32, gestor);

    assert(res == false);
    assert(gestor.tieneErrores());
    assert(gestor.obtenerTodos()[0].codigo == CodigosDiagnostico::kRegistroNoSoportado64Bit);

    // Fuente con registro no soportado
    GestorDiagnosticos gestor2;
    Instruccion instFuenteInvalida = AnalizadorSintactico::analizarTexto("add rax, r99", 2);
    assert(Validador::validarInstruccion(instFuenteInvalida, gestor2) == false);

    std::cout << "  -> Exito en rechazo de registros desconocidos.\n";
}

void probarOperandosInvertidos() {
    std::cout << "[Test Gate] Probando rechazo de operandos invertidos (E014)...\n";

    GestorDiagnosticos gestor;
    // Constante como primer operando (destino inmediato)
    Instruccion instInvertida = AnalizadorSintactico::analizarTexto("mov 42, rax", 3);
    bool res = Validador::validarInstruccion(instInvertida, gestor);

    assert(res == false);
    assert(gestor.tieneErrores());
    assert(gestor.obtenerTodos()[0].codigo == CodigosDiagnostico::kFormaOperandoNoSoportada);

    // Destino inmediato en operacion logica
    GestorDiagnosticos gestorXor;
    Instruccion instXorInvertida = AnalizadorSintactico::analizarTexto("xor 42, rax", 4);
    assert(Validador::validarInstruccion(instXorInvertida, gestorXor) == false);
    assert(gestorXor.obtenerTodos()[0].codigo == CodigosDiagnostico::kFormaOperandoNoSoportada);

    std::cout << "  -> Exito en rechazo de operandos invertidos.\n";
}

void probarExcesoOperandos() {
    std::cout << "[Test Gate] Probando rechazo de exceso de operandos (E022)...\n";

    // 1. Tres operandos en add
    GestorDiagnosticos gestor1;
    Instruccion instTresOps = AnalizadorSintactico::analizarTexto("add rax, rbx, rcx", 4);
    assert(Validador::validarInstruccion(instTresOps, gestor1) == false);
    assert(gestor1.obtenerTodos()[0].codigo == CodigosDiagnostico::kExcesoOperandos);

    // 2. Operando en ret
    GestorDiagnosticos gestor2;
    Instruccion instRetConOp = AnalizadorSintactico::analizarTexto("ret rax", 5);
    assert(Validador::validarInstruccion(instRetConOp, gestor2) == false);
    assert(gestor2.obtenerTodos()[0].codigo == CodigosDiagnostico::kExcesoOperandos);

    std::cout << "  -> Exito en rechazo de exceso de operandos.\n";
}

void probarFaltaOperandos() {
    std::cout << "[Test Gate] Probando rechazo por falta de operandos (E023)...\n";

    GestorDiagnosticos gestor;
    Instruccion instUnOp = AnalizadorSintactico::analizarTexto("mov rax", 6);
    assert(Validador::validarInstruccion(instUnOp, gestor) == false);
    assert(gestor.obtenerTodos()[0].codigo == CodigosDiagnostico::kFaltanOperandos);

    std::cout << "  -> Exito en rechazo por falta de operandos.\n";
}

void probarInstruccionDesconocida() {
    std::cout << "[Test Gate] Probando rechazo de instruccion desconocida (E001)...\n";

    GestorDiagnosticos gestor;
    Instruccion instDesconocida = AnalizadorSintactico::analizarTexto("vmovaps ymm0, ymm1", 7);
    assert(Validador::validarInstruccion(instDesconocida, gestor) == false);
    assert(gestor.obtenerTodos()[0].codigo == CodigosDiagnostico::kInstruccionDesconocida);

    std::cout << "  -> Exito en rechazo de instruccion desconocida.\n";
}

int main() {
    std::cout << "=== Pruebas Gate del Validador Semantico ===\n";
    probarCasosValidos();
    probarRegistrosDesconocidos();
    probarOperandosInvertidos();
    probarExcesoOperandos();
    probarFaltaOperandos();
    probarInstruccionDesconocida();
    std::cout << "Todas las condiciones Gate del validador pasaron con exito.\n";
    return 0;
}
