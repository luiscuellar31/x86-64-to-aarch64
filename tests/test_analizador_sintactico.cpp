#include <iostream>
#include <cassert>
#include "analizador_sintactico.hpp"

void probarAnalisisInstruccionesBasicas() {
    std::cout << "[Test] Probando analisis de instrucciones basicas (mov, add, sub, ret)...\n";

    // 1. add rax, 5
    Instruccion instAdd = AnalizadorSintactico::analizarTexto("add rax, 5", 4);
    assert(instAdd.codigoOperacion == CodigoOperacion::Add);
    assert(instAdd.numeroLinea == 4);
    assert(instAdd.operandos.size() == 2);
    assert(instAdd.operandos[0].tipo == TipoOperando::Registro);
    assert(instAdd.operandos[0].valor == "rax");
    assert(instAdd.operandos[1].tipo == TipoOperando::Inmediato);
    assert(instAdd.operandos[1].valor == "5");

    // 2. mov rbx, rax
    Instruccion instMov = AnalizadorSintactico::analizarTexto("MOV RBX, RAX", 5);
    assert(instMov.codigoOperacion == CodigoOperacion::Mov);
    assert(instMov.operandos.size() == 2);
    assert(instMov.operandos[0].tipo == TipoOperando::Registro);
    assert(instMov.operandos[0].valor == "rbx");
    assert(instMov.operandos[1].tipo == TipoOperando::Registro);
    assert(instMov.operandos[1].valor == "rax");

    // 3. sub rax, 0x10
    Instruccion instSub = AnalizadorSintactico::analizarTexto("sub rax, 0x10", 6);
    assert(instSub.codigoOperacion == CodigoOperacion::Sub);
    assert(instSub.operandos.size() == 2);
    assert(instSub.operandos[1].tipo == TipoOperando::Inmediato);
    assert(instSub.operandos[1].valor == "0x10");

    // 4. ret
    Instruccion instRet = AnalizadorSintactico::analizarTexto("  RET  ", 7);
    assert(instRet.codigoOperacion == CodigoOperacion::Ret);
    assert(instRet.operandos.empty());

    std::cout << "  -> Exito en analisis de instrucciones basicas.\n";
}

void probarAnalisisEtiquetas() {
    std::cout << "[Test] Probando analisis de etiquetas y combinaciones...\n";

    // 1. Etiqueta pura
    Instruccion instEtiqueta = AnalizadorSintactico::analizarTexto("inicio:", 1);
    assert(instEtiqueta.tieneEtiqueta());
    assert(instEtiqueta.esEtiquetaPura());
    assert(instEtiqueta.etiqueta == "inicio");

    // 2. Etiqueta con instruccion acompañante
    Instruccion instCombinada = AnalizadorSintactico::analizarTexto("bucle: ADD RAX, 1", 10);
    assert(instCombinada.tieneEtiqueta());
    assert(instCombinada.etiqueta == "bucle");
    assert(instCombinada.codigoOperacion == CodigoOperacion::Add);
    assert(instCombinada.operandos.size() == 2);
    assert(instCombinada.operandos[0].valor == "rax");
    assert(instCombinada.operandos[1].valor == "1");

    std::cout << "  -> Exito en analisis de etiquetas.\n";
}

void probarAnalisisProgramaCompleto() {
    std::cout << "[Test] Probando analisis de programa completo...\n";

    std::string codigo =
        "; Prueba de programa\n"
        "inicio:\n"
        "    mov rax, 10\n"
        "    add rax, 5\n"
        "    ret\n";

    std::vector<Instruccion> programa = AnalizadorSintactico::analizarPrograma(codigo);
    assert(programa.size() == 4);

    assert(programa[0].esEtiquetaPura());
    assert(programa[0].etiqueta == "inicio");
    assert(programa[0].numeroLinea == 2);

    assert(programa[1].codigoOperacion == CodigoOperacion::Mov);
    assert(programa[1].numeroLinea == 3);

    assert(programa[2].codigoOperacion == CodigoOperacion::Add);
    assert(programa[2].numeroLinea == 4);

    assert(programa[3].codigoOperacion == CodigoOperacion::Ret);
    assert(programa[3].numeroLinea == 5);

    std::cout << "  -> Exito en analisis de programa completo.\n";
}

void probarAnalisisOperandosMemoria() {
    std::cout << "[Test] Probando analisis de operandos de memoria estructurados...\n";

    // 1. mov rax, [rbp - 8]
    Instruccion instLoad = AnalizadorSintactico::analizarTexto("mov rax, [rbp - 8]", 1);
    assert(instLoad.codigoOperacion == CodigoOperacion::Mov);
    assert(instLoad.operandos.size() == 2);
    assert(instLoad.operandos[0].esRegistro() && instLoad.operandos[0].valor == "rax");
    assert(instLoad.operandos[1].esMemoria());
    assert(instLoad.operandos[1].memoria.registroBase == "rbp");
    assert(instLoad.operandos[1].memoria.desplazamiento == -8);
    assert(instLoad.operandos[1].memoria.anchoBits == 64);

    // 2. mov [rbp - 16], rbx
    Instruccion instStore = AnalizadorSintactico::analizarTexto("mov [rbp - 16], rbx", 2);
    assert(instStore.codigoOperacion == CodigoOperacion::Mov);
    assert(instStore.operandos.size() == 2);
    assert(instStore.operandos[0].esMemoria());
    assert(instStore.operandos[0].memoria.registroBase == "rbp");
    assert(instStore.operandos[0].memoria.desplazamiento == -16);
    assert(instStore.operandos[1].esRegistro() && instStore.operandos[1].valor == "rbx");

    // 3. mov rax, [rbp + 32]
    Instruccion instPositivo = AnalizadorSintactico::analizarTexto("mov rax, [rbp + 32]", 3);
    assert(instPositivo.operandos[1].esMemoria());
    assert(instPositivo.operandos[1].memoria.desplazamiento == 32);

    // 4. mov rax, [rbp] (desplazamiento cero)
    Instruccion instCero = AnalizadorSintactico::analizarTexto("mov rax, [rbp]", 4);
    assert(instCero.operandos[1].esMemoria());
    assert(instCero.operandos[1].memoria.desplazamiento == 0);

    // 5. mov rax, [rbp-8] (sin espacios)
    Instruccion instCompacto = AnalizadorSintactico::analizarTexto("mov rax, [rbp-8]", 5);
    assert(instCompacto.operandos[1].esMemoria());
    assert(instCompacto.operandos[1].memoria.desplazamiento == -8);

    // 6. clasificarOperando individual
    Operando opMem = AnalizadorSintactico::clasificarOperando("[rbp - 24]");
    assert(opMem.esMemoria());
    assert(opMem.memoria.registroBase == "rbp");
    assert(opMem.memoria.desplazamiento == -24);

    std::cout << "  -> Exito en analisis de operandos de memoria.\n";
}

void probarAnalisisStack() {
    std::cout << "[Test] Probando analisis de instrucciones de stack (push, pop)...\n";

    // 1. push rbp
    Instruccion instPush = AnalizadorSintactico::analizarTexto("push rbp", 1);
    assert(instPush.codigoOperacion == CodigoOperacion::Push);
    assert(instPush.esStack());
    assert(instPush.operandos.size() == 1);
    assert(instPush.operandos[0].esRegistro());
    assert(instPush.operandos[0].valor == "rbp");

    // 2. pop rbp
    Instruccion instPop = AnalizadorSintactico::analizarTexto("pop rbp", 2);
    assert(instPop.codigoOperacion == CodigoOperacion::Pop);
    assert(instPop.esStack());
    assert(instPop.operandos.size() == 1);
    assert(instPop.operandos[0].esRegistro());
    assert(instPop.operandos[0].valor == "rbp");

    // 3. push rbx y pop rbx
    Instruccion instPushRbx = AnalizadorSintactico::analizarTexto("push rbx", 3);
    assert(instPushRbx.codigoOperacion == CodigoOperacion::Push);
    assert(instPushRbx.operandos[0].valor == "rbx");

    Instruccion instPopRbx = AnalizadorSintactico::analizarTexto("pop rbx", 4);
    assert(instPopRbx.codigoOperacion == CodigoOperacion::Pop);
    assert(instPopRbx.operandos[0].valor == "rbx");

    std::cout << "  -> Exito en analisis de push y pop.\n";
}

void probarAnalisisLlamadas() {
    std::cout << "[Test] Probando analisis de instruccion call...\n";

    Instruccion instCall = AnalizadorSintactico::analizarTexto("call calcular_suma", 1);
    assert(instCall.codigoOperacion == CodigoOperacion::Call);
    assert(instCall.esLlamada());
    assert(instCall.operandos.size() == 1);
    assert(instCall.operandos[0].esEtiqueta());
    assert(instCall.operandos[0].valor == "calcular_suma");

    std::cout << "  -> Exito en analisis de call.\n";
}

int main() {
    std::cout << "=== Pruebas unitarias de AnalizadorSintactico ===\n";
    probarAnalisisInstruccionesBasicas();
    probarAnalisisEtiquetas();
    probarAnalisisProgramaCompleto();
    probarAnalisisOperandosMemoria();
    probarAnalisisStack();
    probarAnalisisLlamadas();
    std::cout << "Todas las pruebas del analizador sintactico pasaron con exito.\n";
    return 0;
}
