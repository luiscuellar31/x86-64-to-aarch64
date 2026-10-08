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

int main() {
    std::cout << "=== Pruebas unitarias de AnalizadorSintactico (Fase 2, Subfase 2) ===\n";
    probarAnalisisInstruccionesBasicas();
    probarAnalisisEtiquetas();
    probarAnalisisProgramaCompleto();
    std::cout << "Todas las pruebas del analizador sintactico pasaron con exito.\n";
    return 0;
}
