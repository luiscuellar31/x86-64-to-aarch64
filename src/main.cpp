#include <iostream>
#include "instruccion.hpp"
#include "normalizador.hpp"
#include "analizador_sintactico.hpp"
#include "ir_aarch64.hpp"
#include "diagnostico.hpp"

int main() {
    std::cout << "=== Demostracion del Traductor x86-64 a AArch64 ===\n\n";

    // 1. Demostracion del modelo de datos de instruccion (Fase 0)
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

    // 3. Demostracion del analizador sintactico lineal (Fase 2, Subfases 2 y 3)
    std::cout << "\n[3] Analisis sintactico y lexico (Fase 2, Subfases 2 y 3):\n";
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

    // 4. Demostracion de la Representacion Intermedia (IR) AArch64 (Fase 2, Subfase 4)
    std::cout << "\n[4] Representacion Intermedia AArch64 (Fase 2, Subfase 4):\n";
    InstruccionAArch64 irAdd(CodigoOperacionAArch64::Add, 3);
    irAdd.agregarOperando(OperandoAArch64::crearRegistro("x0"));
    irAdd.agregarOperando(OperandoAArch64::crearRegistro("x0"));
    irAdd.agregarOperando(OperandoAArch64::crearInmediato(5));
    irAdd.reglaId = "ARITH_ADD_IMM";
    irAdd.explicacion = "x86 usa destino destructivo de 2 operandos; AArch64 requiere 3 operandos explicitos";

    std::cout << "  Instruccion IR destino:   " << irAdd.emitirTexto() << "\n";
    std::cout << "  Regla aplicada:           " << irAdd.reglaId << "\n";
    std::cout << "  Explicacion didactica:    " << irAdd.explicacion << "\n";

    // 5. Demostracion de Diagnosticos Estructurados (Fase 2, Subfase 5)
    std::cout << "\n[5] Diagnosticos Estructurados (Fase 2, Subfase 5):\n";
    GestorDiagnosticos gestor;
    gestor.agregarError(CodigosDiagnostico::kInstruccionDesconocida, 4, "instruccion 'vmovaps' no soportada", 5);
    gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, 7, "registro 'eax' no soportado en modo solo 64 bits", 9);
    gestor.agregarAdvertencia("W001", 1, "directiva de ensamblador ignorada");

    std::cout << "  Total diagnósticos: " << gestor.cantidadTotal() << " (Errores: "
              << gestor.cantidadErrores() << ", Advertencias: " << gestor.cantidadAdvertencias() << ")\n";
    gestor.imprimir();

    std::cout << "\nComprobacion finalizada con exito.\n";
    return 0;
}