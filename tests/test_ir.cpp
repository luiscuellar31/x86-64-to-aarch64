#include <iostream>
#include <cassert>
#include "instruccion.hpp"
#include "ir_aarch64.hpp"

void probarSemanticaOperandoX86() {
    std::cout << "[Test] Probando semantica de Operando x86-64...\n";

    // 1. Registro
    Operando operandoRegistro = Operando::crearRegistro("rax");
    assert(operandoRegistro.esRegistro());
    assert(!operandoRegistro.esInmediato());
    assert(!operandoRegistro.esEtiqueta());
    assert(operandoRegistro.valor == "rax");

    // 2. Inmediato decimal positivo
    Operando inmediatoPositivo = Operando::crearInmediato("100");
    assert(inmediatoPositivo.esInmediato());
    assert(inmediatoPositivo.valorNumerico == 100);

    // 3. Inmediato decimal negativo
    Operando inmediatoNegativo = Operando::crearInmediato("-50");
    assert(inmediatoNegativo.esInmediato());
    assert(inmediatoNegativo.valorNumerico == -50);

    // 4. Inmediato hexadecimal
    Operando inmediatoHexadecimal = Operando::crearInmediato("0x1F");
    assert(inmediatoHexadecimal.esInmediato());
    assert(inmediatoHexadecimal.valorNumerico == 31);

    // 5. Etiqueta
    Operando operandoEtiqueta = Operando::crearEtiqueta("bucle");
    assert(operandoEtiqueta.esEtiqueta());
    assert(!operandoEtiqueta.esRegistro());

    std::cout << "  -> Exito en semantica de Operando x86-64.\n";
}

void probarEmisionIRDeAArch64() {
    std::cout << "[Test] Probando creacion y emision de IR AArch64...\n";

    // 1. add x0, x0, #5
    InstruccionAArch64 instruccionAdd(CodigoOperacionAArch64::Add, 4);
    instruccionAdd.agregarOperando(OperandoAArch64::crearRegistro("x0"));
    instruccionAdd.agregarOperando(OperandoAArch64::crearRegistro("x0"));
    instruccionAdd.agregarOperando(OperandoAArch64::crearInmediato(5));
    instruccionAdd.reglaId = "ARITH_ADD_IMM";
    instruccionAdd.explicacion = "Transformacion x86 a AArch64 con 3 operandos explicitos";

    assert(instruccionAdd.emitirTexto() == "add x0, x0, #5");
    assert(instruccionAdd.reglaId == "ARITH_ADD_IMM");
    assert(instruccionAdd.lineaOrigen == 4);

    // 2. mov x19, x0
    InstruccionAArch64 instruccionMov(CodigoOperacionAArch64::Mov, 5);
    instruccionMov.agregarOperando(OperandoAArch64::crearRegistro("x19"));
    instruccionMov.agregarOperando(OperandoAArch64::crearRegistro("x0"));
    assert(instruccionMov.emitirTexto() == "mov x19, x0");

    // 3. ret
    InstruccionAArch64 instruccionRet(CodigoOperacionAArch64::Ret, 6);
    assert(instruccionRet.emitirTexto() == "ret");

    // 4. Etiqueta pura
    InstruccionAArch64 instruccionEtiqueta(CodigoOperacionAArch64::Etiqueta, 1);
    instruccionEtiqueta.etiqueta = "inicio";
    assert(instruccionEtiqueta.esEtiquetaPura());
    assert(instruccionEtiqueta.emitirTexto() == "inicio:");

    std::cout << "  -> Exito en construccion y emision de IR AArch64.\n";
}

void probarMemoriaIR() {
    std::cout << "[Test] Probando operandos de memoria en IR x86-64 y AArch64...\n";

    // 1. Operando x86-64 memoria negativa
    Operando memoriaNegativa = Operando::crearMemoria("rbp", -8);
    assert(memoriaNegativa.esMemoria());
    assert(memoriaNegativa.memoria.registroBase == "rbp");
    assert(memoriaNegativa.memoria.desplazamiento == -8);
    assert(memoriaNegativa.memoria.anchoBits == 64);
    assert(memoriaNegativa.aTexto() == "Memoria([rbp - 8])");

    // 2. Operando x86-64 memoria positiva
    Operando memoriaPositiva = Operando::crearMemoria("rbp", 16);
    assert(memoriaPositiva.aTexto() == "Memoria([rbp + 16])");

    // 3. Operando x86-64 memoria cero
    Operando memoriaSinDesplazamiento = Operando::crearMemoria("rbp", 0);
    assert(memoriaSinDesplazamiento.aTexto() == "Memoria([rbp])");

    // 4. Instruccion AArch64 ldr x0, [x29, #-8]
    InstruccionAArch64 instruccionLdr(CodigoOperacionAArch64::Ldr, 10);
    instruccionLdr.agregarOperando(OperandoAArch64::crearRegistro("x0"));
    instruccionLdr.agregarOperando(OperandoAArch64::crearMemoria("x29", -8));
    assert(instruccionLdr.emitirTexto() == "ldr x0, [x29, #-8]");

    // 5. Instruccion AArch64 str x19, [x29, #-16]
    InstruccionAArch64 instruccionStr(CodigoOperacionAArch64::Str, 11);
    instruccionStr.agregarOperando(OperandoAArch64::crearRegistro("x19"));
    instruccionStr.agregarOperando(OperandoAArch64::crearMemoria("x29", -16));
    assert(instruccionStr.emitirTexto() == "str x19, [x29, #-16]");

    // 6. Instruccion AArch64 ldr x0, [x29] (desplazamiento cero)
    InstruccionAArch64 instruccionLdrCero(CodigoOperacionAArch64::Ldr, 12);
    instruccionLdrCero.agregarOperando(OperandoAArch64::crearRegistro("x0"));
    instruccionLdrCero.agregarOperando(OperandoAArch64::crearMemoria("x29", 0));
    // 7. Instruccion AArch64 str con pre-indexado: str x29, [sp, #-16]!
    InstruccionAArch64 instruccionPre(CodigoOperacionAArch64::Str, 13);
    instruccionPre.agregarOperando(OperandoAArch64::crearRegistro("x29"));
    instruccionPre.agregarOperando(OperandoAArch64::crearMemoria("sp", -16, ModoIndexadoAArch64::PreIndexado));
    assert(instruccionPre.emitirTexto() == "str x29, [sp, #-16]!");

    // 8. Instruccion AArch64 ldr con post-indexado: ldr x29, [sp], #16
    InstruccionAArch64 instruccionPost(CodigoOperacionAArch64::Ldr, 14);
    instruccionPost.agregarOperando(OperandoAArch64::crearRegistro("x29"));
    instruccionPost.agregarOperando(OperandoAArch64::crearMemoria("sp", 16, ModoIndexadoAArch64::PostIndexado));
    assert(instruccionPost.emitirTexto() == "ldr x29, [sp], #16");

    // 9. Instruccion AArch64 bl (branch with link / llamada a subrutina)
    InstruccionAArch64 instruccionBl(CodigoOperacionAArch64::Bl, 15);
    instruccionBl.agregarOperando(OperandoAArch64::crearEtiqueta("mi_subrutina"));
    assert(instruccionBl.emitirTexto() == "bl mi_subrutina");

    std::cout << "  -> Exito en pruebas de memoria, modos de indexado y bl en IR.\n";
}

int main() {
    std::cout << "=== Pruebas unitarias de IR / Representacion Intermedia ===\n";
    probarSemanticaOperandoX86();
    probarEmisionIRDeAArch64();
    probarMemoriaIR();
    std::cout << "Todas las pruebas de la IR pasaron con exito.\n";
    return 0;
}
