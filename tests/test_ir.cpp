#include <iostream>
#include <cassert>
#include "instruccion.hpp"
#include "ir_aarch64.hpp"

void probarSemanticaOperandoX86() {
    std::cout << "[Test] Probando semantica de Operando x86-64...\n";

    // 1. Registro
    Operando opReg = Operando::crearRegistro("rax");
    assert(opReg.esRegistro());
    assert(!opReg.esInmediato());
    assert(!opReg.esEtiqueta());
    assert(opReg.valor == "rax");

    // 2. Inmediato decimal positivo
    Operando opImmPos = Operando::crearInmediato("100");
    assert(opImmPos.esInmediato());
    assert(opImmPos.valorNumerico == 100);

    // 3. Inmediato decimal negativo
    Operando opImmNeg = Operando::crearInmediato("-50");
    assert(opImmNeg.esInmediato());
    assert(opImmNeg.valorNumerico == -50);

    // 4. Inmediato hexadecimal
    Operando opImmHex = Operando::crearInmediato("0x1F");
    assert(opImmHex.esInmediato());
    assert(opImmHex.valorNumerico == 31);

    // 5. Etiqueta
    Operando opEtiq = Operando::crearEtiqueta("bucle");
    assert(opEtiq.esEtiqueta());
    assert(!opEtiq.esRegistro());

    std::cout << "  -> Exito en semantica de Operando x86-64.\n";
}

void probarEmisionIRDeAArch64() {
    std::cout << "[Test] Probando creacion y emision de IR AArch64...\n";

    // 1. add x0, x0, #5
    InstruccionAArch64 instAdd(CodigoOperacionAArch64::Add, 4);
    instAdd.agregarOperando(OperandoAArch64::crearRegistro("x0"));
    instAdd.agregarOperando(OperandoAArch64::crearRegistro("x0"));
    instAdd.agregarOperando(OperandoAArch64::crearInmediato(5));
    instAdd.reglaId = "ARITH_ADD_IMM";
    instAdd.explicacion = "Transformacion x86 a AArch64 con 3 operandos explicitos";

    assert(instAdd.emitirTexto() == "add x0, x0, #5");
    assert(instAdd.reglaId == "ARITH_ADD_IMM");
    assert(instAdd.lineaOrigen == 4);

    // 2. mov x19, x0
    InstruccionAArch64 instMov(CodigoOperacionAArch64::Mov, 5);
    instMov.agregarOperando(OperandoAArch64::crearRegistro("x19"));
    instMov.agregarOperando(OperandoAArch64::crearRegistro("x0"));
    assert(instMov.emitirTexto() == "mov x19, x0");

    // 3. ret
    InstruccionAArch64 instRet(CodigoOperacionAArch64::Ret, 6);
    assert(instRet.emitirTexto() == "ret");

    // 4. Etiqueta pura
    InstruccionAArch64 instEtiqueta(CodigoOperacionAArch64::Etiqueta, 1);
    instEtiqueta.etiqueta = "inicio";
    assert(instEtiqueta.esEtiquetaPura());
    assert(instEtiqueta.emitirTexto() == "inicio:");

    std::cout << "  -> Exito en construccion y emision de IR AArch64.\n";
}

int main() {
    std::cout << "=== Pruebas unitarias de IR / Representacion Intermedia (Fase 2, Subfase 4) ===\n";
    probarSemanticaOperandoX86();
    probarEmisionIRDeAArch64();
    std::cout << "Todas las pruebas de la IR pasaron con exito.\n";
    return 0;
}
